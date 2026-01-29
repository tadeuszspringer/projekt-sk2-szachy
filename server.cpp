#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chessgame.h"

void * gameThread(void *arg) {
    GameSession *session = (GameSession *)arg;
    int whiteSock = session->whiteSocket;
    int blackSock = session->blackSocket;
    delete session;

    ChessGame game;
    char buffer[1024];
    int n;
    
    bool updateOpponent = true;
    std::string errorMessage = "";

    send(whiteSock, "Grasz Bialymi (Gracz 1).\n", 32, 0);
    send(blackSock, "Grasz Czarnymi (Gracz 2).\n", 33, 0);

    while (true) {
        int currentSock = (game.currentTurn == 0) ? whiteSock : blackSock;
        int otherSock = (game.currentTurn == 0) ? blackSock : whiteSock;

        if (game.isGameOver()) {
            std::string finalBoard = game.getBoardString();
            std::string winMsg = "\nWYGRALES\n";
            std::string loseMsg = "\nPRZEGRALES\n";

            if (game.lost_white) {
                send(blackSock, (finalBoard + winMsg).c_str(), (finalBoard + winMsg).length(), 0);
                send(whiteSock, (finalBoard + loseMsg).c_str(), (finalBoard + loseMsg).length(), 0);
            } 
            else if (game.lost_black) {
                send(whiteSock, (finalBoard + winMsg).c_str(), (finalBoard + winMsg).length(), 0);
                send(blackSock, (finalBoard + loseMsg).c_str(), (finalBoard + loseMsg).length(), 0);
            }
            
            printf("Koniec gry - MAT.\n");
            break;
        }

        std::string board = game.getBoardString();
        std::string instructions = "\nWpisz: skad dokad (np. 'e 6 e 4')\n";
        std::string turnMsg = (game.currentTurn == 0) ? "Tura BIALYCH > " : "Tura CZARNYCH > ";
        
        std::string fullMsgCurrent;
        if (!errorMessage.empty()) {
            fullMsgCurrent = "\n " + errorMessage + " \n" + board + instructions + turnMsg;
        } else {
            fullMsgCurrent = board + instructions + turnMsg;
        }

        send(currentSock, fullMsgCurrent.c_str(), fullMsgCurrent.length(), 0);

        if (updateOpponent) {
            std::string fullMsgOther = board + "\nCzekaj na ruch przeciwnika...\n";
            send(otherSock, fullMsgOther.c_str(), fullMsgOther.length(), 0);
        }

        errorMessage = ""; 

        memset(buffer, 0, 1024);
        n = recv(currentSock, buffer, 1024, 0);
        
        if (n <= 0) {
            send(otherSock, "Przeciwnik sie rozlaczyl. Wygrales walkowerem.\n", 48, 0);
            break;
        }

        char c1, c2; 
        int r1, r2;
        
        if (sscanf(buffer, " %c %d %c %d", &c1, &r1, &c2, &r2) == 4) {
            int x1 = c1 - 'a'; 
            int x2 = c2 - 'a';
            if (x1 < 0) x1 = c1 - 'A';
            if (x2 < 0) x2 = c2 - 'A';
            int y1 = r1;
            int y2 = r2;

            if (game.makeMove(y1, x1, y2, x2, game.currentTurn)) {
                updateOpponent = true;
                send(currentSock, "Ruch poprawny.\n", 15, 0);
            } else {
                updateOpponent = false;
                errorMessage = "Ruch niemozliwy (zla figura, kolizja lub szach)";
            }
        } else {
            updateOpponent = false;
            errorMessage = "Zly format! Uzyj: litera liczba litera liczba";
        }
    }

    close(whiteSock);
    close(blackSock);
    pthread_exit(NULL);
}

int main(){
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    int opt = 1; 
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if(listen(serverSocket, 50)==0)
        printf("Serwer nasluchuje na porcie 1100\n");
    else
        printf("Blad bindingu\n");

    int waitingSocket = -1; 

    while(1) {
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        
        if (newSocket < 0) continue;

        printf("Nowe polaczenie\n");

        if (waitingSocket == -1) {
            waitingSocket = newSocket;
            send(waitingSocket, "Czekanie na drugiego gracza\n", 32, 0);
        } else {
            printf("Start gry\n");
            pthread_t thread_id;
            GameSession *session = new GameSession;
            session->whiteSocket = waitingSocket;
            session->blackSocket = newSocket;
            
            if( pthread_create(&thread_id, NULL, gameThread, session) != 0 )
                printf("Blad tworzenia watku\n");
            
            pthread_detach(thread_id);
            waitingSocket = -1; 
        }
    }
    return 0;
}