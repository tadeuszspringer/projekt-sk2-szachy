#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    if (argc < 3) {
        printf("Podaj <port> <adres_serwera>\n");
        return 1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        printf("Błędny numer portu.\n");
        return 1;
    }
    char message[1000];
    char buffer[2048];
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[2]);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    addr_size = sizeof serverAddr;
    if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) < 0) {
        printf("Blad polaczenia\n");
        return 1;
    }
    
    printf("Polaczono. Czekaj na gre\n");

    for(;;){
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));

        if(recv(clientSocket, buffer, 2048, 0) <= 0) {
            printf("Polaczenie zakonczone\n");
            break;
        }
        
        printf("%s", buffer);

        if (strstr(buffer, "SZACH MAT") != NULL) {
            printf("\nKONIEC GRY\n");
            break;
        }
        if (strstr(buffer, "Wygrales walkowerem") != NULL) {
            break;
        }

        if (strstr(buffer, "Tura") != NULL && strstr(buffer, "Czekaj") == NULL) 
        {
            if (fgets(message, sizeof(message), stdin) == NULL) break; 

            if(strncmp(message, "exit", 4) == 0) {
                printf("Wychodzenie\n");
                break;
            }

            if( send(clientSocket , message , strlen(message) , 0) < 0) {
                printf("Blad wysylania\n");
                break;
            }
        }
    }

    close(clientSocket);
    return 0;
}
