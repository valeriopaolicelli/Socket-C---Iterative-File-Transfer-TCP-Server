Socket C - Iterative File Transfer TCP Server

*********** Riassunto: Versione italiana (english below):

Come lavora (ambiente UNIX):

essenzialmente ci sono due oggetti, il client e il server, strutture dati che contengono tutte le informazioni necessarie per instaurare
un socket e permettere l'invio di uno o più file dalla directory del server, verso il client.

Il client riceve da linea di comando due parametri: ./client <server address> <port number> .
Richiede uno o più file, come da protocollo specificato nel file Assignment.txt e attende di riceverli e salvarli nella sua directory.

Il server riceve da linea di comando un parametro: ./server <port number> .
Attende che arrivino richieste su quella porta, per instaurare la connessione con i client e procedere come specificato nel protocollo
indicato nel file Assignment.txt .

Ci sono due versioni di server TCP, uno sequenziale che accetta solo un client per volta, ed uno concorrente, capace di servire più client
contemporaneamete.

I sorgenti presenti per fare ciò sono:
    Libreria -> Creare oggetto client:
        - myclient.c
        - myclient.h
    Richiamo alle funzioni per creare, connettere e far operare un client:
        - client1_main.c

    Libreria -> Creare oggetto server:
        - myserver.c
        - myserver.h
    Richiamo alle funzioni per creare, connettere e far operare un server:
        - server1_main.c (server sequenziale)
        - server2_main.c (server concorrente)

Per la gestione del socket si è fatto riferimento alle funzioni del testo  ' Stevens, Unix network programming (3ed) ':
    - sockwrap.c
    - sockwrap.h
    - errlib.c
    - errlib.h

Per la gestione dei codici di errore, si è creata una libreria per una tabella di simboli, array che per ogni indice contiene un messaggio d'errore:
    - sterr.c
    - sterr.h

Per ulteriori chiarimenti sulle singole funzioni, consultare i commenti nel codice.

********** SUMMARY (English version): 

Overview: How it works (UNIX environment):

Mainly there are two objects representing the client and the server, which contain the data structures with all informations about the socket 
(like address and port number), to send some files from the server to the receiver clients.

The client receives from command line two parameters: ./client <server address> <port number> .
It asks for one or more files of the server directory, as indicated in the protocol (see the file Assignment.txt).
So, it waits to receive and to store them into its own directory.

The server receives from command line only one parameters: ./server <port number> .
It waits on the specified port number, the client requests, to setup the TCP connection and proceed as specified in the protocol (see the file Assignment.txt).

Overall, there are two TCP server versions, the first one is sequential to accept only one client per time, while the other one is concurrent.

The source to do this are:
    My C library -> to create the client object:
        - myclient.c
        - myclient.h
    The main -> Functions usefull to build / connect / handle the client: 
        - client1_main.c

    My C library -> to create the client object (its the same for the two versions of the server):
        - myserver.c
        - myserver.h
    The main -> Functions usefull to build / connect / handle the server: 
        - server1_main.c (sequential server)
        - server2_main.c (concurrent server)

To manage the socket, I've used the functions of the tesxt book ' Stevens, Unix network programming (3ed) ':
    - sockwrap.c
    - sockwrap.h
    - errlib.c
    - errlib.h

To handle the error codes, I've build a C library with a symbol table, which contains an array where each index is referred to a message error:
    - sterr.c
    - sterr.h

To retrieve further informatios, see the comments inside the sources.

                                                                                                                                        Typed by Valerio Paolicelli, Turin 17th November, 2018.