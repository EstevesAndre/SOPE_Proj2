#ifndef _REQUEST_H
#define _REQUEST_H

typedef struct
{
        int client_id;
        int n_seats;
        int array_cnt;
        int error_status;
        int* seats;
} request;

#endif