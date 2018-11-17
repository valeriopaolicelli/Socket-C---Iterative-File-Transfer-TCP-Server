          /*******************************************************************/
 	     /**SOURCE CODE FOR WRAPPER OF SOCKET FUNCTIONS			            */ 
        /***Purpose: library of wrapper and utility socket functions       */
       /****         wrapper functions include error management           */
      /*****Author: Valerio Paolicelli - Matricola: 253054               */
     /******Reference: Stevens, Unix network programming (3ed)          */
    /*******Assignment of course "Distribuited Programming I"          */
   /********Master Degree in Computer Engineering - Data Science      */
  /*********Politecnico di Torino - 21/05/2018                       */
 /**********                                                        */
/*******************************************************************/
#include <stdlib.h> // getenv()
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> // timeval
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_aton()
#include <sys/un.h> // unix sockets
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h> // SCNu16
#include <fcntl.h>
#include "errlib.h"
#include "sockwrap.h"

extern char *prog_name;

int Socket (int family, int type, int protocol)
{
	int n;
	if ( (n = socket(family,type,protocol)) < 0)
		err_sys ("(%s) error - socket() failed", prog_name);
	return n;
}

void Bind (int sockfd, const SA *myaddr,  socklen_t myaddrlen)
{
	if ( bind(sockfd, myaddr, myaddrlen) != 0)
		err_sys ("(%s) error - bind() failed", prog_name);
}

void Listen (int sockfd, int backlog)
{
	char *ptr;
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);
	if ( listen(sockfd,backlog) < 0 )
		err_sys ("(%s) error - listen() failed", prog_name);
}


int Accept (int listen_sockfd, SA *cliaddr, socklen_t *addrlenp)
{
	int n;
again:
	if ( (n = accept(listen_sockfd, cliaddr, addrlenp)) < 0)
	{
		if (INTERRUPTED_BY_SIGNAL ||
			errno == EPROTO || errno == ECONNABORTED ||
			errno == EMFILE || errno == ENFILE ||
			errno == ENOBUFS || errno == ENOMEM			
		    )
			goto again;
		else
			err_sys ("(%s) error - accept() failed", prog_name);
	}
	return n;
}

int connect_nonb (int sockfd, const SA *saptr, socklen_t salen, int nsec){
	int flags, error, n;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags= fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	error= 0;
	if((n= connect(sockfd,saptr, salen))<0)
		if(errno!= EINPROGRESS)
			return -1;

    if( n== 0)
    	goto done;

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset= rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;
	
	if((n = Select(sockfd +1, &rset, &wset, NULL, nsec ? &tval : NULL)) == 0){
		close(sockfd);
		errno= ETIMEDOUT;
		return -1;
	}

	if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)){
		len= sizeof(error);
		if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)< 0)
			return -1;
	}
	else
		err_quit("Select error: sockfd not set");

	done:
		fcntl(sockfd, F_SETFL, flags);
		if(error){
			close(sockfd);
			errno= error;
			return -1;
		}
	return 0;
}

/* reads exactly "n" bytes from a descriptor */
ssize_t readn (int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nread = read(fd, ptr, nleft)) < 0)
		{
			if (INTERRUPTED_BY_SIGNAL)
			{
				nread = 0;
				continue; /* and call read() again */
			}
			else
				return -1;
		}
		else
			if (nread == 0)
				break; /* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return n - nleft;
}


ssize_t sendn (int fd, const void *vptr, size_t n, int flags)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nwritten = send(fd, ptr, nleft, flags)) <= 0)
		{
			if (INTERRUPTED_BY_SIGNAL)
			{
				nwritten = 0;
				continue; /* and call send() again */
			}
			else
				return -1;
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return n;
}


ssize_t writen (int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (INTERRUPTED_BY_SIGNAL)
			{
				nwritten = 0;
				continue; /* and call write() again */
			}
			else
				return -1;
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return n;
}

int Select (int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
	int n;
again:
	if ( (n = select (maxfdp1, readset, writeset, exceptset, timeout)) < 0)
	{
		if (INTERRUPTED_BY_SIGNAL)
			goto again;
		else
			err_sys ("(%s) error - select() failed", prog_name);
	}
	return n;
}

void
showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
    printf("%s %s!",str,p);
    printf("%" SCNu16, ntohs(a->sin_port));
    printf("\n");
}

int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
        return 1;
  
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}