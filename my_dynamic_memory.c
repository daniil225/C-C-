#include <stdio.h>
#include <string.h>

const int MAX_POOLS = 1000;
const long BUF_SIZE = 104888320;

static char ALLBUF[104888320]; // Достуаная пямять
static unsigned long AVAIBLE = BUF_SIZE; // Свободно
static char* pools[1000]; // Пулы
static unsigned int POOLS_COUNT = 1; // Количество пулов
static unsigned int pools_size[1000]; // Размеры пулов

static char* blocks[1000]; // Блоки
static unsigned int BLOCK_COUNT = 0; // количество существующих блоков
static unsigned int block_size[1000]; // Размер блоков
static unsigned int RIGHT_BLOCK; // Номер самого правого блока

// --- обработка ошибок ---
static int ARS_ALLOC_ERR = 0; // Флаг для ошибок
#define NO_MEMORY 1
#define BLOCK_NOT_FOUND 2

void ars_alloc_init(void); // Инициализация
char* ars_malloc(unsigned long Size); // Выделения
int ars_free(char* block); // Освобождение
int ars_defrag(void); // дефрагментация
void ars_alloc_start(char *str); // статистика
int GetArsAllocError(void); // Возврат кода последей ошибки

//================================================================================
void ars_alloc_init(void)
{
    pools[0] = ALLBUF; // Начало первого пула
    pools_size[0] = AVAIBLE;
}

//===============================================================================
char* ars_malloc(unsigned long Size)
{
    unsigned long i,k;
    char* p;
    // Если требуется больше чем есть
    if(Size > AVAIBLE)
    {
        ARS_ALLOC_ERR = NO_MEMORY;
        return 0;
    }
     //---
     p = 0;
     // Ищем первый подходящий пул
     for(i = 0; i < POOLS_COUNT; ++i)
        if(Size <= pools_size[i])
        {
            p = pools[i]; // Запоминаем адрес
            k = i;  // ... и номер
            break;
        }

    if(!p)  // Если подходящий пул не найден
    {
        ARS_ALLOC_ERR = NO_MEMORY;
        return 0;
    }

    blocks[BLOCK_COUNT] = p; // Здесь будет новый блок
    block_size[BLOCK_COUNT] = Size;
    ++BLOCK_COUNT;
    ++RIGHT_BLOCK;
    pools[k] = (char*)(p + Size + 1); // Смещаем адрес начала пула на конец блока
    pools_size[k] = pools_size[k] - Size; // и исправляем размер

    AVAIBLE -= Size; // Места стало меньше
    return p;
}


int ars_free(char* block)
{
    unsigned int i,k;
    char *p = 0;
    // Ищем блок по адресу
    for(i = 0; i < RIGHT_BLOCK; ++i)
        if(block == blocks[i])
        {
            p = blocks[i];
            k = i;
            break;
        }
    
    if(!p)
    {
        ARS_ALLOC_ERR = BLOCK_NOT_FOUND;
        return BLOCK_NOT_FOUND;
    }

    blocks[k] = 0;
    --BLOCK_COUNT;
    pools[POOLS_COUNT] = block; // Превращаем блок в пул
    pools_size[POOLS_COUNT] = block_size[k];
    ++POOLS_COUNT;
    AVAIBLE += block_size[k]; // Месьа стало больше

    return 0;
}


// Функция дефрагментации. Смещает все блоки влево и остовляет единственный пул
int ars_defrag(void)
{
    unsigned int i,k;
    char* p = ALLBUF;
    char* t, *tmp;

    for(i = 0; i < RIGHT_BLOCK; ++i)
    {
        t = blocks[i];
        if(t == ALLBUF)
        {
            p = (char*)(blocks[i] + block_size[i] + 1);
            continue;
        }
        tmp = p;
        for(k = 0, t = blocks[i]; k < block_size[i]; ++k)
            *p++ = *t++;
        blocks[i] = tmp;
    }

    POOLS_COUNT = 1;
    pools[0] = p;
    AVAIBLE = BUF_SIZE - (unsigned long)(p - ALLBUF);
    pools_size[0] = AVAIBLE;
    RIGHT_BLOCK = 0;
    return 0;
}

// Вывод статистики
void ars_alloc_stat(char *str)
{
    printf("Statistic:\nAvaible: %ld of %ld bytes\nCount of block: %d\n",AVAIBLE, BUF_SIZE, BLOCK_COUNT);
}

int GetArsAllocError(void)
{
    return ARS_ALLOC_ERR;
}



int main()
{
    ars_alloc_init();
    char *p = ars_malloc(20);
    blocks[0] = "wljnljfnlsjdnsljdnsljnlsjnjsssc";
    ars_free(p);
    ars_defrag();
    printf("%s", blocks[0]);
    ars_alloc_stat(p);
    return 0;
}