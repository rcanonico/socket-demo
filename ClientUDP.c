// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Client UDP
// Sintassi: ClientUDP [host] [port]
//			host: nome/indirizzo del computer sul quale il server 
//				e' in esecuzione
//			port: numero di port da utilizzare
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Librerie
#include <sys/socket.h>
#include <netinet/in.h> // struct sockaddr_in
#include <netdb.h>	// gethostbyname()

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 	// atoi()
#include <unistd.h> 	// close()

// Costanti
#define PORTserver 5194
#define PORTclient 5195

int main(int argc, char *argv[])
{
	struct sockaddr_in sad;		// SERVER
	struct sockaddr_in cad;		// CLIENT
	struct hostent *ptrh;		// struttura per memorizzare la risoluzione del nome del server
	int    sd;			// descrittore della socket
	unsigned short port;		// numero di port del servizio
	char   buf[1000];		// buffer in cui memorizzare la stringa
	
	char localhost[] = "localhost";
	char *host;
	
	// Lettura delle opzioni a riga di comando
	// Numero di port
	if (argc > 2) {
		port = atoi(argv[2]);
		if (port == 0) {
			fprintf(stderr, "numero di port non valido\n");
			return 1;
		}
	} else {
		port = PORTserver;
	}

	// Indirizzo del server
	if (argc > 1)
		host = argv[1];
	else
		host = localhost;

	// Conversione del nome in indirizzo IP
	if ((ptrh = gethostbyname(host)) == NULL) {
		fprintf(stderr, "host non valido %s\n", host);
		return 1;
	}

	// Inzializzazione della struttura sad (Server)
	memset((char *)&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);	// copia dell'indirizzo nel campo sin_addr della struttura sad
	sad.sin_port = htons(port);
	
	// Inizializzazione della struttura cad (Client)
	memset((char *)&cad, 0, sizeof(cad));
	cad.sin_family = AF_INET;
	//cad.sin_port = htons((u_short)PORTclient);

	// Creazione di una socket di tipo SOCK_DGRAM - UDP
	if ((sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "socket creation failed\n");
		return 1;
	}
	
	// Configurazione dell'endpoint locale della socket utilizzando 
	// i parametri della struttura sockaddr_in cad
	if (bind(sd, (struct sockaddr *)&cad, sizeof(cad)) < 0) {
		fprintf(stderr, "bind fallito\n");
		exit(1);
	}

	// Ciclo infinito
	while (1) {
		// Richiesta di una stringa all'utente
		memset(buf, 0, sizeof(buf));
		printf("Scrivi una stringa da spedire al server: ");
		fgets(buf, sizeof(buf), stdin);

		// Invio della stringa al server
		sendto(sd, buf, strlen(buf),0,(struct sockaddr*)&sad, sizeof(sad));

		// Chiusura della connessione con la stringa "quit"
		if (!strncmp(buf, "quit", 4)) {
			break;
		} 

		// Ricezione della risposta dal server
		memset(buf, 0, sizeof(buf));
		recv(sd, buf, sizeof(buf), 0);
		printf("Ricevuto dal server: %s\n", buf);
	}

	// Chiusura della socket in ascolto
	close(sd);
	
	return 0;
}


