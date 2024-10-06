// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ServerTCP - server iterativo - v_2020 (Roberto Canonico)
// VERSIONE PER LINUX
// Sintassi: ServerTCP [port]
//    port: numero di port TCP del server
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Codice fornito solo a scopo illustrativo
// Non garantito esente da bug ...
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Librerie
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     // atoi()
#include <unistd.h>     // close()
#include <stdbool.h>    // bool type

// Costanti
#define DEFAULT_SERVER_PORT 5194
#define SERVER_BUFFER_SIZE 51
#define QLEN 0

int main(int argc, char *argv[])
{
    struct sockaddr_in sad;           // struct sockaddr con l'indirizzo ed il port number del server
    int    sd;                        // descrittore della socket di ascolto
    unsigned short port;              // numero di port del servizio
    char request[SERVER_BUFFER_SIZE]; // buffer per memorizzare la richiesta
    char   reply[SERVER_BUFFER_SIZE]; // buffer per memorizazre la risposta
	
    // Lettura delle opzioni a riga di comando
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port == 0) {
            fprintf(stderr, "numero di port non valido\n");
            return 1;
        }
    } else {
        port = DEFAULT_SERVER_PORT;
    }

    // Inizializzazione della struttura sad
    memset((char *)&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = INADDR_ANY;
    // conversione di port da "host byte order" a "TCP/IP network order"
    sad.sin_port = htons(port);        
    
    // Creazione di una socket di tipo SOCK_STREAM - TCP
    if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "creazione della socket fallita\n");
        return 1;
    }

    // Configurazione dell'endpoint locale della socket utilizzando 
    // i parametri della struttura sockaddr_in sad
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr, "bind fallito\n");
        return 1;
    }

    // Avvio dell'ascolto sulla socket sd
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr, "listen fallito\n");
        exit(1);
    }
    
    // Ciclo infinito
    while (true) {
        int nsd;                         // socket per la comunicazione con il client
        struct sockaddr_in cad;          // struct sockaddr con l'indirizzo ed il port number del client
        unsigned int len = sizeof(cad);
        int n_received;
        
        // Attesa di una connessione TCP
        printf("Server in ascolto su port TCP %d\n", port);
        if ((nsd = accept(sd, (struct sockaddr *) &cad, &len)) < 0) {
            fprintf(stderr, "accept fallito\n");
            return 1;
        }

        printf("Connesso a client %s:%d\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
        while (true){
            // Ricezione della request inviata dal client
            memset(request, 0, SERVER_BUFFER_SIZE);
            printf("Aspetto dati dal client....\n");
            n_received = read(nsd, &request, SERVER_BUFFER_SIZE-1);
            if (n_received == 0) {
                printf("Il client ha chiuso la connessione.\n");
                break;
            }
               
            printf("Ricevuto dal client %d bytes: <%s>\n", n_received, request);

            // Operazione di elaborazione della request e produzione della reply
            printf("Elaborazione in corso...\n");
            for (int i = 0; request[i] != 0; i++)
                reply[i] = (request[i] >= 'a' && request[i] <= 'z') ? (request[i] - 32) : request[i];
            reply[n_received] = '\0';

            // Invio della reply al client
            write(nsd, &reply, strlen(reply));
            printf("Risposta inviata al client: <%s>\n", reply);

        }
       
        // Chiusura della socket relativa alla connessione
        printf("Chiusura connessione con client: %s:%d\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
        close(nsd);
        printf("----------------------------------------------------------\n");
       
    }

    // Chiusura della socket in ascolto
    close(sd);
}

