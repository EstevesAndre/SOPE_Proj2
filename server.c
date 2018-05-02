#include <stdio.h> 
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include <string.h>
#include <pthread.h> 
#include <unistd.h> 
#include "server.h"
#include "constants.h"
#include "request.h"

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
request* buffer;
int occupied_seats;

int main(int argc, char *argv[])
{
time_t start = time(NULL);
int n_seats, n_offices, open_time;

server_argchk(argc, argv, &n_seats, &n_offices, &open_time);

int seats[n_seats];
initSeats(seats, n_seats);

pthread_t offices[n_offices];

buffer = NULL;
occupied_seats = 0;

int i;
for (i=0; i<n_offices; i++) 
{
    pthread_create(&offices[i], NULL, NULL, seats);
}

int requests = createRequestFifo();
request r;

while(1) {
    char* info = malloc(100 * sizeof(char));
    read(requests, info, 100);
    parseRequest(&r, info, n_seats);

    while(buffer != NULL)
    {
        if (time(NULL) > start + open_time)
        {
            printf("Timeout. Server closing.\n");
            close(requests);
            exit(0); 
        }
    }

    buffer = &r;

    if (time(NULL) > start + open_time) 
    {
        printf("Timeout. Server closing.\n");
        close(requests);
        exit(0); 
    }
}
}

void server_argchk(int argc, char* argv[], int* n_seats, int* n_offices, int* open_time)
{
    if(argc == 1)
    {
        printf("Usage: %s <num_room_seats> <num_ticket_offices> <open_time>\n", argv[0]);
        exit(0);
    }
    if(argc != 4)
    {
        printf("Invalid number of arguments: %s\n", argv[0]);
        exit(1);
    }
    else
    {
        *n_seats = strtol(argv[1], NULL, 10);

        if(*n_seats <= 0 || *n_seats > MAX_ROOM_SEATS)
        {
            printf("Invalid argument: %s\n", argv[1]);
            exit(1);
        } 

        *n_offices = strtol(argv[2], NULL, 10);

        if(*n_seats <= 0)
        {
            printf("Invalid argument: %s\n", argv[2]);
            exit(1);
        } 

        *open_time = strtol(argv[2], NULL, 10);

        if(*open_time <= 0)
        {
            printf("Invalid argument: %s\n", argv[3]);
            exit(1);
        }  
    }
}

void initSeats(int seats[], int n_seats)
{
    int i;
    for(i = 0; i < n_seats; i++)
    {
        seats[0] = 0;
    }
}

int createRequestFifo()
{
    int i = mkfifo("requests",0660);
    if(i != 0)
    {
        printf("Failed to create requests fifo\n");
        exit(2);
    }

    int fd = open("requests",O_RDONLY);
    if(fd == -1)
    {
        printf("Failed to create requests fifo\n");
        exit(2);
    }

    return fd;
}

void parseRequest(request* r, char* info, int n_seats)
{
    char * delim_1 = strtok(info, " ");
    if(delim_1 == NULL)
        {
            r->error_status = REQ_ERR_PARAM_OTHER;
            return;
        }
    r->client_id = strtol(info, &delim_1, 10);
    if(r->client_id == 0)
        {
            r->error_status = REQ_ERR_PARAM_OTHER;
            return;
        }

    char * delim_2 = strtok(NULL, " ");
    if(delim_2 == NULL)
        {
            r->error_status = REQ_ERR_PARAM_OTHER;
            return;
        }
    r->n_seats = strtol(delim_1, &delim_2, 10);
    if(r->n_seats == 0)
        {
            r->error_status = REQ_ERR_PARAM_OTHER;
            return;
        }

    delim_1 = delim_2;
    delim_2 = strtok(NULL, " ");

    r->array_cnt = 0;

    while(delim_2 != NULL)
    {
        int i = strtol(delim_1, &delim_2, 10);
        if(i == 0)
        {
            r->error_status = REQ_ERR_PARAM_OTHER;
            return;
        }
        r->array_cnt++;
        r->seats = realloc(r->seats, r->array_cnt * sizeof(int));
        r->seats[r->array_cnt - 1] = i;
        delim_1 = delim_2;
        delim_2 = strtok(NULL, " ");
    }

    int i = strtol(delim_1, NULL, 10);
    r->array_cnt++;
    r->seats = realloc(r->seats, r->array_cnt * sizeof(int));
    r->seats[r->array_cnt - 1] = i;

    requestErrorChk(r, n_seats);
}

void requestErrorChk(request* r, int n_seats)
{
    if(occupied_seats = n_seats)
    {
        r->error_status = REQ_ERR_ROOM_FULL;
        return;
    }

    if(r->n_seats > MAX_CLI_SEATS)
    {
        r->error_status = REQ_ERR_OVER_MAX_SEATS;
        return;
    }

    if(r->n_seats > r->array_cnt)
    {
        r->error_status = REQ_ERR_UNDER_SEAT_ID;
        return;
    }

    int i;
    for(i = 0; i < r->array_cnt; i++)
    {
        if(r->seats[i] < 1 || r->array_cnt > n_seats)
        {
            r->error_status = REQ_ERR_SEAT_ID_INV;
            return;
        }
    }


}