// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// SERVER UDP iterativo
// Sintassi: ServerUDP [port]
//			 port: numero di port da utilizzare
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Librerie
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 	// atoi()
#include <unistd.h> 	// close()

// Costanti
#define PORT 5194

int main(int argc, char *argv[])
{
	struct sockaddr_in sad;		// SERVER
	struct sockaddr_in cad;		// CLIENT
	int    sd;			// descrittore della socket
	unsigned short port;		// numero di port del servizio
	char   buf[1000];		// buffer in cui memorizzare la stringa
	socklen_t cadlen = sizeof(cad);	// dimensione della struttura cad

	// Lettura delle opzioni a riga di comando
	if (argc > 1) {
		port = atoi(argv[1]);
		if (port == 0) {
			fprintf(stderr, "numero di port non valido\n");
			return 1;
		}
	} else {
		port = PORT;
	}

	// Inzializzazione della struttura sad
	memset((char *)&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = INADDR_ANY;
	sad.sin_port = htons(port);		// conversione di port da "host byte order" a "TCP/IP network order"

	// Creazione di una socket di tipo SOCK_DGRAM - UDP
	if ((sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "creazione della socket fallita\n");
		return 1;
	}

	// Configurazione dell'endpoint locale della socket utilizzando 
	// i parametri della struttura sockaddr_in sad
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr, "bind fallito\n");
		return 1;
	}
	printf("Server in ascolto sul port UDP %d\n", port);
	
	// Ciclo infinito
	while (1) 
	{
		int i;
		char src[1000];
		
		// Ricezione della stringa inviata dal client
		memset(buf, 0, sizeof(buf));
  		recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&cad, &cadlen);
		printf("Ricevuto dal client: %s\n", buf);

		// Operazione di conversione sulla stringa ricevuta
		printf("Elaborazione in corso...\n");
		for (i = 0; buf[i] != 0; i++)
			src[i] = (buf[i] >= 97 && buf[i] <= 122) ? (buf[i] - 32) : buf[i];
		src[i] = '\0';

		// Simulazione del tempo di elaborazione
		sleep(3);

		// Invio della stringa convertita al client
		sendto(sd, src, strlen(buf),0,(struct sockaddr*) &cad, sizeof(cad));
		printf("Inviata al client: %s\n", src);
	}

	// Chiusura della socket in ascolto
 	close(sd);
}

