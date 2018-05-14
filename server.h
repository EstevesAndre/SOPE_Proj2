#ifndef _SERVER_H
#define _SERVER_H

#include <pthread.h>

#include "request.h"
#include "constants.h"


void server_argchk(int argc, char* argv[], int* n_seats, int* n_offices, int* open_time);
void initSeats(int seats[], int n_seats);
FILE* createRequestFifo();
void parseRequest(request* r, char* info, int n_seats);
void requestErrorChk(request* r, int n_seats);
void *office_f(void *nr);
void requestHandle(Seat* seats, request* r);
int isSeatFree(Seat *seats, int seatNum);
void bookSeat(Seat *seats, int seatNum, int clientId);
void freeSeat(Seat *seats, int seatNum);
void sendMessagetoClient(request *r, int error_status, char* msg, char* msg2log);

#endif
