// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ServerTCPc - server concorrente - v_2020 (Roberto Canonico)
// VERSIONE PER LINUX
// Sintassi: ServerTCPc [port]
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
#define QLEN 2
#define MAX_SOCKETS 20

int main(int argc, char *argv[])
{
    struct sockaddr_in sad;    // SERVER
    int  sd;        // descrittore della socket principale
    unsigned short port;    // numero di port del servizio
    char buf[1000];        // buffer in cui memorizzare la stringa
    int csd[MAX_SOCKETS];    // vettore di descrittori per le connessioni
    int maxsd;        // descrittore piu' grande
    int lastsd_idx;        // indice dell'ultimo descrittore utilizzato nel vettore csd
    fd_set allset;        // set di descrittori da monitorare con la select
    fd_set rset;        // set di desctittori restituito dalla select
    struct timeval time_out;// valore di timeout per la select
    int i;

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

    // Inzializzazione della struttura sad
    memset((char *)&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = INADDR_ANY;
    sad.sin_port = htons(port);        // conversione di port da "host byte order" a "TCP/IP network order"
    
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
    printf("Server in ascolto sul port TCP %d\n", port);

    // Inizializzazione del set di socket allset e delle variabili ausiliarie
    FD_ZERO(&allset);
    FD_SET(sd, &allset);
    maxsd = sd;
    lastsd_idx = 0;

    // Inizializzazione del vettore dei descrittori per le connessioni
    for (i=0; i< MAX_SOCKETS; i++) {
        csd[i] = -1;
    }

    // Ciclo infinito
    while (1) {
        int nsd;     // nuova socket relativa all'ultima connessione
        int ready_sds;    // numero di socket che hanno generato un evento

        // Aggiornamento dei parametri per la select
        rset = allset;
        time_out.tv_sec = 5;
        time_out.tv_usec = 0;
        
        // Attesa di un evento sui descrittori contenuti in allset
        if ((ready_sds = select(maxsd + 1, &rset, NULL, NULL, &time_out)) < 0) {
            perror("select fallita");
            exit(1);
        }
        
        // Timeout scaduto
        if (ready_sds == 0)
            continue;
        
        // Ricezione di una nuova connessione
        if (FD_ISSET(sd, &rset)) {
            // Attesa di una connessione TCP (senza attesa)
            if ((nsd = accept(sd, NULL, NULL)) < 0) {
                fprintf(stderr, "accept fallito\n");
                exit(1);
            } else {
                // Ricerca del primo descrittore libero in csd
                for (i = 0; i < MAX_SOCKETS; i++) {
                    if (csd[i] == -1) {
                        csd[i] = nsd;        // Aggiunta del descrittore al vettore
                        if (i > lastsd_idx) 
                            lastsd_idx = i;    // Aggiornamento dell'ultimo descrittore utilizzato
                        if (nsd > maxsd) 
                            maxsd = nsd;    // Aggiornamento del descrittore piu' grande
                        FD_SET(nsd, &allset);    // Aggiunta del nuovo descrittore al set allset
                        break;
                    }
                }

                // Controllo sul numero massimo di socket gestibili
                if (i == MAX_SOCKETS) {
                    printf("socket %d rifiutato\n", i);
                    close(nsd);    // Chiusura forzata della nuova socket 
                } else {
                    printf("socket %d accettato\n", i);
                }
            }
            
            // Fine del ciclo se non vi sono altri eventi in attesa
            if ( --ready_sds <= 0)
                continue;
        }

        // Gestione delle comunicazioni attive che hanno generato un evento
        for (i = 0; i <= lastsd_idx; i++) {

            // Ignora i descrittori inutilizzati
            if (csd[i] == -1) 
                continue;
            
            // Controllo del descrittore i-esimo
            if (FD_ISSET(csd[i], &rset)) {
                char src[1000];    // buffer per la stringa modificata
                int j;

                // Ricezione della stringa inviata dal client
                memset(buf, 0, sizeof(buf));
                read(csd[i], &buf, sizeof(buf));
                printf("Client %d: %s\n", i, buf);

                // Chiusura della connessione da parte del client con la stringa "quit"
                if (!strncmp(buf, "quit", 4)) {
                    close(csd[i]);            // Chiusura della socket
                    FD_CLR(csd[i], &allset);    // Rimozione del descrittore dal set allset
                    csd[i] = -1;            // Rimozione del descrittore dal vettore csd
                    printf("socket %d chiuso\n", i);
                    continue;
                }
                    
                // Operazione di conversione sulla stringa ricevuta
                printf("Elaborazione in corso...\n");
                for (j = 0; buf[j] != 0; j++)
                    src[j] = (buf[j] >= 97 && buf[j] <= 122) ? (buf[j] - 32) : buf[j];
                src[j] = '\0';

                // Simulazione del tempo di elaborazione
                sleep(0.5);

                // Invio della stringa convertita al client
                write(csd[i], &src, strlen(src));
                printf("Client %d: %s\n", i, src);
                
                // Fine del ciclo se non vi sono altri eventi in attesa
                if(--ready_sds <= 0)
                    break;
            }
        }
        
        // Reinizializzazione maxsd e maxsd_idx
        maxsd = csd[0];
        lastsd_idx = 0;
        for (i = 0; i < MAX_SOCKETS; i++) {
            if (csd[i] > maxsd)
                maxsd = csd[i];
            if (csd[i] != -1 && i > lastsd_idx)
                lastsd_idx = i;
        }
        if (maxsd == -1)     // tutte le connessioni sono state chiuse
            maxsd = sd;
    }

    // Chiusura della socket in ascolto
     close(sd);
}

