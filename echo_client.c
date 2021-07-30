#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

/*Реализация паккета RIO*/
#define RIO_BUFSIZE 8192
#define MAXLINE 8192
#define LISTENQ 1024

typedef struct sockaddr SA;


/*Rio - пакет функций устойчивых -ввода - вывода*/

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

/*Функции для работы с сетью*/
int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in  serveraddr;

    if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
    
    if((hp = gethostbyname(hostname)) == NULL)
        return -2;
    
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char*) hp->h_addr, (char *)&serveraddr.sin_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    if(connect(clientfd, (SA*) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

int open_listenfd(int port)
{
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    if((listenfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
        return -1;
    
    if(setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR, (const void*)&optval, sizeof(int)) < 0)
        return -1;
    
    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);
    if(bind(listenfd, (SA*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    
    if(listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
}

int main(int argc, char **argv)
{
    int clientfd, port;

    char *host, buf[MAXLINE];

    rio_t rio;

    if(argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = atoi(argv[2]);
    clientfd = open_clientfd(host, port);
    rio_readinitb(&rio, clientfd);

    while(fgets(buf, MAXLINE, stdin) != NULL)
    {
        rio_writen(clientfd,buf, strlen(buf));
        rio_readlineb(&rio, buf, MAXLINE);
        fputs(buf, stdout);
    }
    close(clientfd);
    exit(0);
}