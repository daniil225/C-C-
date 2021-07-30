//#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <stddef.h>
/*Основные константы макрооопределения*/
#define WSIZE 4 /*Размер слова в байтах*/
#define DSIZE 8 /*Размер двойного слова в байтах*/
#define CHUNKSIZE (1<<12) /*начальный размер динамической памяти в байтах*/
#define OVERHEAD 8 /*размер заголовков и поля признаков в байтах*/
#define MAX(x,y)((x) > (y) ? (x) : (y))
/*Упаковывает в слово размер и разряд распределения*/
#define PACK(size, alloc)((size) | (alloc))
/*Читает и записывает слово по адресу p*/
#define GET(p) (*(size_t*)(p))
#define PUT(p,val) (*(size_t*)(p) = (val))
/*Читает размер и распределенные поля по адресу p*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
/*Для заданного указателя bp вычисляет адрес его заголовка сверху и заголовка снизу*/
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
/*для заданного указателя блока bp вычисляет адрес следующего и предыдущего блоков*/
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))


static char mem_start_brk;/*Указывает на первоначальный байт памяти*/
static char mem_brk;/*Указывает на последний байт данной памяти*/
static char mem_max_addr; /*максимальный адрес втрьуальной памяти*/
static char* heap_listp;
void mem_init(int size)
{
    mem_start_brk = (char)malloc(size);
    mem_brk = mem_start_brk;
    mem_max_addr = mem_start_brk + size;
}

void *mem_sbrk(int incr)
{
    char *old_brk = mem_brk;
    if((incr < 0) || ((mem_brk + incr) > mem_max_addr))
    {
        errno = ENOMEM;
        return (void*)-1;
    }

    mem_brk += incr;
    return old_brk;
}

/*Инициализация динамичесской памяти*/
int mm_init(void)
{
    /*Образуем исходную пустую динамическую память*/
    if((heap_listp = mem_sbrk(4*WSIZE)) == NULL)
        return -1;
    PUT(heap_listp,0); /*Дополнение дял выравнивания*/
    PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1)); /*заголовок сверху пролога*/
    PUT(heap_listp + WSIZE+DSIZE, PACK(OVERHEAD,1)); /*Заголовок снизу пролога*/
    PUT(heap_listp+WSIZE +DSIZE, PACK(0,1)); /*Заголовок сверху эпилога*/
    heap_listp += DSIZE;
    /*Расширяем пустую динамическую память со свободным блоком размером CHUNKSIZE байтов*/
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc)
        return bp;
    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    }
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        return(PREV_BLKP(bp));
    }
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        return(PREV_BLKP(bp));

    }
}
/*Расширение динамической памяти*/
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    
    /*выделяем четное число слов с целью соответствия требованиям выравнивания*/
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((int)(bp = mem_sbrk(size)) < 0)
        return NULL;
    
    /*Инициализирует заголовок сверху снизу свободного блока и заголовок сверху эпилога*/
    PUT(HDRP(bp), PACK(size, 0)); /*Заголовок сверху свободного блока*/
    PUT(FTRP(bp), PACK(size,0)); /*Заголовок снизу свободного блока*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); /*Заголовок снизу новгого эпилога*/

    /*Слияние если предыдущий блок был свободен*/
    return coalesce(bp);
}




/*Метод граничных тегов*/
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

static void *find_fit(size_t asize)
{
    void *bp;
    /*Поиск по методу первого подходящего*/
    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL; /*Подходящих нет*/
}

/*Размещает запрошенный блок в начале свободного блока и разбивающего его только в случае если размер избыточной равняется или превышает минимальный размер блока*/
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if((csize - asize) >= (DSIZE + OVERHEAD))
    {
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if(size <= 0)
        return NULL;
    
    /*Коректируем размер блока включая байты дополнения и выравнивания*/
    if(size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else 
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) /DSIZE);
    
    /*Поиск подходящего блока в списке свбодных блоков*/
    if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    /*Подходящий блок не найден получить дополнительную память и разместить блок*/
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

int main()
{
 
    return 0;
}