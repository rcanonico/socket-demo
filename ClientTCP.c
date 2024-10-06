// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ClientTCP - v_2020 (Roberto Canonico)
// VERSIONE PER LINUX
// Sintassi: ClientTCP [server_host] [server_port]
//    server_host: nome o indirizzo IP del server
//    server_port: numero di port del servizio sul server
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Codice fornito solo a scopo illustrativo
// Non garantito esente da bug ...
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Librerie
#include <sys/socket.h>
#include <netinet/in.h> // struct sockaddr_in
#include <netdb.h>      // gethostbyname()

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     // atoi()
#include <unistd.h>     // close()
#include <stdbool.h>

// Costanti
#define DEFAULT_SERVER_PORT 5194 
#define CLIENT_BUFFER_SIZE 101
#define QUIT "QUIT"

int main(int argc, char *argv[])
{
    struct sockaddr_in sad;           // struct sockaddr con l'indirizzo ed il port number del server
    struct hostent *ptrh;             // struttura per memorizzare la risoluzione del nome del server
    int    sd;                        // descrittore della socket usata dal client
    char*  server_host;               // nome o indirizzo IP del server
    unsigned short server_port;       // numero di port del servizio sul server
    char request[CLIENT_BUFFER_SIZE]; // buffer per memorizzare la richiesta
    char   reply[CLIENT_BUFFER_SIZE]; // buffer per memorizzare la risposta    int    request_len, reply_len;  // lunghezza in byte dei messaggi request (inviato) e reply (atteso)
    int    request_len;               // lunghezza in byte del messaggio inviato (request)
    int    reply_len;                 // lunghezza in byte del messaggio atteso  (reply)
    int    n_sent;                    // numero di byte inviati  con una singola operazione send
    int    total_sent;                // numero di byte della request inviati fino ad un certo momento
    int    n_received;                // numero di byte ricevuti con una singola operazione recv
    int    total_received;            // numero di byte della reply ricevuti fino ad un certo momento
    bool   session_ended = false;     // se true la sessione con il server è terminata e la connessione è chiusa
    char   localhost[] = "localhost";
	
    // Lettura delle opzioni a riga di comando
    // Indirizzo del server
    if (argc > 1)
        server_host = argv[1];
    else
        server_host = localhost;

    // Numero di port
    if (argc > 2) {
        server_port = atoi(argv[2]);
        if (server_port == 0) {
            fprintf(stderr, "numero di port non valido\n");
            return 1;
        }
    } else {
        server_port = DEFAULT_SERVER_PORT;
    }

    // Conversione del nome in indirizzo IP
    if ((ptrh = gethostbyname(server_host)) == NULL) {
        fprintf(stderr, "server_host non valido %s\n", server_host);
        return 1;
    }

    // Inzializzazione della struttura sad
    memset((char *)&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    // copia dell'indirizzo del server nel campo sin_addr della struttura sad
    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
    // conversione di server_port da "host byte order" a "TCP/IP network order"
    sad.sin_port = htons(server_port);
    
    // Creazione di una socket di tipo SOCK_STREAM - TCP
    if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "errore nella creazione della socket\n");
        return 1;
    }
    
    // Connessione al server (three-way handshake TCP)
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr, "connect fallita\n");
        return 1;
    }
    
    // Ciclo nel quale il client rimane finchè session_ended non diventa true
    while (!session_ended) {
        // Richiesta di una stringa all'utente
        memset(request, 0, CLIENT_BUFFER_SIZE);
        printf("Scrivi una stringa da inviare al server: ");
        fgets(request, CLIENT_BUFFER_SIZE, stdin);
        // La stringa letta con fgets termina con il carattere newline prima del carattere NUL
        // Sostituisci il carattere newline con il carattere NUL
        request[strcspn(request, "\n")] = '\0';
        request_len = strlen(request);
        if (request_len == 0) {
            printf("Devi inserire una stringa non vuota...\n");
            continue;
        }
        // Uscita dal ciclo con la stringa "."
        if (!strncmp(request, QUIT, strlen(QUIT))) {
            printf("Hai inserito la stringa di terminazione. Il client terminerà.\n");
            session_ended = true;
            break;
        } 

        // Invio della stringa al server
        total_sent = 0;
        while (total_sent < request_len) {
            n_sent = write(sd, request+total_sent, request_len-total_sent);
            printf("Inviati %d bytes al server....\n", n_sent);
            total_sent += n_sent;
        }
        printf("----> REQUEST = <%s>\n", request);

        printf("----------------------------------------------------------------------------\n");        
        // CONVENZIONE: il messaggio reply deve avere la stessa lunghezza di request
        reply_len = request_len;
         
        // Ricezione della risposta dal server
        memset(reply, 0, CLIENT_BUFFER_SIZE);
        total_received = 0;
        while (total_received < reply_len) {
            printf("Attesi %d bytes dal server.....\n", (reply_len-total_received));
            n_received = read(sd, reply+total_received, CLIENT_BUFFER_SIZE-total_received);
            printf("Ricevuto dal server %d bytes...\n", n_received);
            if (n_received == 0) {
                printf("Il server ha chiuso la connessione.\n");
                session_ended = 1;
                break;
            }
            total_received += n_received;
        }
        printf("----> REPLY = <%s>\n", reply);

    }
    
    // Chiusura della connessione con il server
    close(sd);  
    return 0;
}


