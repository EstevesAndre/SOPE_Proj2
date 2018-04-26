#include "request.h"

void server_argchk(int argc, char* argv[], int* n_seats, int* n_offices, int* open_time);
void initSeats(int seats[], int n_seats);
int createRequestFifo();
void parseRequest(request* r, char* info, int n_seats);
void requestErrorChk(request* r, int n_seats);