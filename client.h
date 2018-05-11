#ifndef _CLIENT_H
#define _CLIENT_H  

void client_argchk(int argc, char* argv[], int* open_time, int* n_seats, char* seats_list);
void timeout(int signo);
void sendMessage(int fd, int n_seats, char* seats_list);
void printReserve(char* reserve);
void printError(int return_status);

#endif