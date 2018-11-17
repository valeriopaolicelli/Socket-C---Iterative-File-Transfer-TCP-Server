 	     /*******************************************************************/
 	    /**HEADER FOR WRAPPER OF SOCKET FUNCTIONS			               */ 
       /***Purpose: library and definitions for socket wrapper            */
      /****Author: Valerio Paolicelli - Matricola: 253054                */
	 /*****Reference: Stevens, Unix network programming (3ed)           */
    /******Assignment of course "Distribuited Programming I"           */
   /*******Master Degree in Computer Engineering - Data Science       */
  /********Politecnico di Torino - 21/05/2018                        */
 /*********                                                         */
/*******************************************************************/
#ifndef _SOCKWRAP_H

#define _SOCKWRAP_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define SA struct sockaddr

#define INTERRUPTED_BY_SIGNAL (errno == EINTR)

int Socket (int family, int type, int protocol);

void Bind (int sockfd, const SA *myaddr,  socklen_t myaddrlen);

void Listen (int sockfd, int backlog);

int Accept (int listen_sockfd, SA *cliaddr, socklen_t *addrlenp);

int connect_nonb (int sockfd, const SA *saptr, socklen_t salen, int nsec);

ssize_t readn (int fd, void *vptr, size_t n);

ssize_t sendn (int fd, const void *vptr, size_t n, int flags);

ssize_t writen (int fd, const void *vptr, size_t n);

int Select (int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);

void showAddr(char *str, struct sockaddr_in *a);

int hostname_to_ip(char * hostname , char* ip);

#endif