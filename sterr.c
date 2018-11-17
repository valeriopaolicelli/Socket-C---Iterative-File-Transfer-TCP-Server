  	    /*****************************************************************************/
 	   /**SOURCE CODE FOR SYMBOL TABLE OF ERROR CODE                                */ 
      /***Purpose: functions and structure of symbol table of error codes/messages */
     /****Author: Valerio Paolicelli - Matricola: 253054               			  */
    /*****Assignment of course "Distribuited Programming I"            			 */
   /******Master Degree in Computer Engineering - Data Science        			*/
  /*******Politecnico di Torino - 21/05/2018                                   */
 /********                                                                    */
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sterr.h"

struct symboltable{
	char **msg; // VECTOR OF STRINGS -> ERROR MESSAGES
	int maxN; // max number of errors
	int n; // current number of error
	int prev_msg; // flag
};

ST init_st(int maxN){ // Initialize the symbol table structure
	ST st= (ST) malloc(sizeof(struct symboltable));
	st->maxN= maxN;
	st->prev_msg= -1; // null
	st->msg=(char**) malloc(st->maxN*sizeof(char *));
	st->msg[0]= strdup("Error code: 0x001 - Server doesn't reply - Connection closed\n");
	st->msg[1]= strdup("Error code: 0x002 - File not found - File name wrong\n");
	st->msg[2]= strdup("Error code: 0x003 - Size of file not found - Connection closed\n");
	st->msg[3]= strdup("Error code: 0x004 - Timestamp of file not found - Connection closed\n");
	st->msg[4]= strdup("Error code: 0x005 - Socket failed\n");
	st->msg[5]= strdup("Error code: 0x006 - Error connecting\n");
	st->msg[6]= strdup("Error code: 0x007 - Invalid address/port\n");
	st->msg[7]= strdup("Error code: 0x008 - Error GET request\n");
	st->msg[8]= strdup("Error code: 0x009 - Error send QUIT command\n");
	st->msg[9]= strdup("Error code: 0x010 - Received: -ERR\nConnection closed by server\n");
	st->msg[10]= strdup("Error code: 0x011 - Timeout expired - Connection closed\n");
	st->msg[11]= strdup("Error code: 0x012 - Error receiving the response - Incomplete file deleted\n");
	st->msg[12]= strdup("Error code: 0x013 - Invalid port number\n");
	st->msg[13]= strdup("Error code: 0x014 - Error fread\n");
	st->msg[14]= strdup("Error code: 0x015 - Error stat system call\n");
	st->msg[15]= strdup("Error code: 0x016 - Error sending +OK\n");
	st->msg[16]= strdup("Error code: 0x017 - Error sending -ERR\n");
	st->msg[17]= strdup("Error code: 0x018 - Error sending size\n");
	st->msg[18]= strdup("Error code: 0x019 - Error sending timestamp\n");
	st->msg[19]= strdup("Error code: 0x020 - Error sending file\n");
	st->msg[20]= strdup("Error code: 0x021 - Error -> Broken Pipe\n");
	st->msg[21]= strdup("Error code: 0x022 - Error -> received unknown message or connection closed by the server\n");
	st->msg[22]= strdup("Error code: 0x023 - Error waiting message from client - Connection closed\n");
	st->msg[23]= strdup("Error code: 0x024 - Error QUIT message - Protocol message doesn't end with CR|LF\n");
	st->msg[24]= strdup("Error code: 0x025 - Error GET request - Protocol message doesn't end with CR|LF\n");
	st->msg[25]= strdup("Error code: 0x026 - Error reading\n");
	st->msg[26]= strdup("Error code: 0x027 - Error writing file (fwrite error) - Incomplete file deleted\n");
	st->n= 27;
	return st;
}

void print_err_msg(ST t, int index){ // print werror message
	printf("%s", t->msg[index]);
	return;
}

int num_error(ST t){ // return number of current errors
	return t->n;
}

/***********RETURNED CODE -> =0 NO ERROR
                          -> OTHERWISE <O -> INDEX TO DIRECT ACCESS TO ST= (CODE * (-1)) - 1 ************/
int check_index(ST t, int index){ // the error code is a negative number, for this it is converted in a positive one,
								  // so the function for printing is called, passing di ST and the trasformed index, decremented by one
							      // (to access to position 0 of ST)
	if(index<0 && t->prev_msg!= SIG_PIPE_CODE){
    	index*=-1;
    	print_err_msg(t, index-1);
    	return -index;
    }
    return 0;
}

void set_prev_msg(ST t, int index){ // set last error occured
	t->prev_msg= index;
}

void free_st(ST t){ // release all
	int i;
	for(i=0;i<t->n;i++){
		free(t->msg[i]);
	}
	free(t->msg);
	free(t);
}