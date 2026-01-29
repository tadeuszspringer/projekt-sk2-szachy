# projekt-sk2-szachy

Projekt wykorzystuje standardowe biblioteki systemowe POSIX oraz C++:
* **iostream, vector, string, cstring** – standardowe biblioteki C++ do logiki gry i operacji na napisach.
* **sys/socket.h, netinet/in.h, arpa/inet.h** – obsługa gniazd i protokołu TCP
* **pthread.h** – wielowątkowość
* **unistd.h, stdio.h, stdlib.h** – funkcje systemowe i wejścia/wyjścia

### Kompilacja
Wymagany jest kompilator `g++` oraz system z rodziny Linux (lub WSL).

```bash
# Kompilacja serwera
g++ -Wall server.cpp -o server

# Kompilacja klienta
g++ -Wall client.cpp -o client
