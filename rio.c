#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

/*Rio - пакет функций устойчивых -ввода - вывода*/
#define RIO_BUFSIZE 8192

/*Устойчивл считывает n байт (небуферизованных)*/
ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0)
    {
        if((nread = read(fd, bufp, nleft)) < 0){
            if(errno = EINTR)
                nread = 0;
            else
                return -1;
        }
        else if(nread == 0)
            break; /*EOF*/
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft); /*>=0*/
}

/*rio_writen - устойчиво записывает n байт (небуферизованных)*/
ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwriten;
    char *bufp = usrbuf;

    while(nleft > 0)
    {
        if((nwriten = write(fd, bufp, nleft)) <= 0)
        {
            if(errno == EINTR)
                nwriten = 0;
            else
                return -1;
        }
        nleft -= nwriten;
        bufp += nwriten;
    }
    return n;
}


typedef struct {
    int rio_fd; /*Дескриптор для даного внутреннего буфера*/
    int rio_cnt;  /*несчитанные байты во внутреннем буфере*/
    char *rio_bufptr; /*следующий несчитанный байт во внутреннем буфере*/
    char rio_buf[RIO_BUFSIZE]; /*внутренний буфер*/
} rio_t;

void rio_readinitb(rio_t *rp, int fd)
{
    rp -> rio_fd = fd;
    rp -> rio_cnt = 0;
    rp -> rio_bufptr = rp -> rio_buf;
}


/*rio_read - упаковщик для считывания Unix read(), переносящий
* минимум (n, rio_cnt) байт из ынутреннего буфера в буфер
* пользователя, где n - количество байт запрошенное пользователем а
* rio_cnt - количество несчитанных байт во внутреннем буфере. При
* вводе записи rio_read() пополняет внутренний буфер по вызову
* функция read(), если внутренний буфер пуст
*/

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while(rp->rio_cnt <= 0) /*Заполнить если буфер пуст*/
    {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            if(errno != EINTR)
                return -1;
            else if(rp->rio_cnt == 0) /*EOF*/
                return 0;
            else
                rp->rio_bufptr = rp->rio_buf;/*Перезагрузка rio_bufptr*/
        }
    }

    cnt  = n;
    if(rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;

    return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for(n = 1; n < maxlen; n++)
    {
        if((rc = rio_read(rp, &c, 1)) == 1){
            *bufp++ = c;
            if(c == '\n')
                break;
        } else if(rc == 0){
            if(n == 1)
                return 0; /*EOF, отсутствие считанных данных*/
            else 
                break; /*EOF, некоторые данные были считаны*/
        }else 
            return -1;/*Ошибка*/
    }
    *bufp = 0;
    return n;
}



int main()
{
    size_t n = 10;
    int r;
    rio_t rio;
    char buf[8192];
    rio_readinitb(&rio, STDIN_FILENO);
    /*while((n = rio_readlineb(&rio, buf, 128)) != 0)
        roi_writen(STDOUT_FILENO, buf, n);
    */
   while ((r = rio_readlineb(&rio, buf, n)) != 0)
   {
       char *c = *rio.rio_buf;
       printf("%s", c);
   }

   return 0;

}
