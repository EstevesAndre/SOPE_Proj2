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

Seat seats[n_seats];
initSeats(seats, n_seats);

pthread_t offices[n_offices];

buffer = NULL;
occupied_seats = 0;

int i;
for (i=0; i<n_offices; i++) 
{
    pthread_create(&offices[i], NULL, office_f, seats);
}

int requests = createRequestFifo();
request r;


while(1) {
    char* info = malloc(100 * sizeof(char));
    int i = read(requests, info, 100);
    if(i != -1)
    {
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
        r.error_status = 0;
    }

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

        *open_time = strtol(argv[3], NULL, 10);

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
    mkfifo("requests",0660);
    int fd = open("requests",O_RDONLY | O_NONBLOCK);
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
    r->client_id = strtol(info, NULL, 10);
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
    r->n_seats = strtol(delim_2, NULL, 10);
    if(r->n_seats == 0)
        {
            r->error_status = REQ_ERR_PARAM_OTHER;
            return;
        }

    delim_1 = delim_2;
    delim_2 = strtok(NULL, " ");

    r->array_cnt = 0;
    r->seats = malloc(0);

    while(delim_2 != NULL)
    {
        int i = strtol(delim_1, NULL, 10);
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
    if(occupied_seats == n_seats)
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

void *office_f(void *nr)
{
   while(1)
   {
       pthread_mutex_lock(&mut);
       if(buffer != NULL)
       {
           request* r = buffer;
           buffer = NULL;
           requestHandle((Seat*) nr, r);
           DELAY();
           pthread_mutex_unlock(&mut);
           continue;
       }
       pthread_mutex_unlock(&mut);
   }
}

void requestHandle(Seat* seats, request* r)
{
    if(r->error_status != 0)
    {
        sendMessagetoClient(r->client_id, r->error_status, NULL);
        return;
    }

    int reserve_cnt = 0;
    int preferences = 0;
    int reserved_seats[MAX_CLI_SEATS + 1] = {0};
    reserved_seats[0] = -1;

    while(reserve_cnt < r->n_seats)
    {
        if(preferences == r->array_cnt)
        {
            int i = 0;
            while(reserved_seats[i] != -1)
            {
                freeSeat(seats, reserved_seats[i]);
                i++;
            }
            sendMessagetoClient(r->client_id, REQ_ERR_UNNAV_SEAT, NULL);
            return;
        }

        if(isSeatFree(seats, *(r->seats + preferences)) == 1)
        {
            bookSeat(seats, *(r->seats + preferences), r->client_id);
            reserved_seats[reserve_cnt] = *(r->seats + preferences);
            reserved_seats[reserve_cnt + 1] = -1;
            reserve_cnt++;
        }

        preferences++;
    }

    char message[1000];
    char aux[8];

    snprintf(message, 1000, "%d", r->n_seats);
    
    int i;
    for(i = 0; i < reserve_cnt; i++)
    {
        snprintf(aux, 8, " %d",reserved_seats[i]);
        strcat(message, aux);
    }

    strcat(message, "\n");
    sendMessagetoClient(r->client_id, REQ_SUCCESSFUL, message);
}

int isSeatFree(Seat *seats, int seatNum)
{
    if(*(seats + seatNum - 1) == 0)
    {
        return 1;
    }

    return 0;
}

void bookSeat(Seat *seats, int seatNum, int clientId)
{
    *(seats + seatNum - 1) = clientId;
}

void freeSeat(Seat *seats, int seatNum)
{
    *(seats + seatNum - 1) = 0;
}

void sendMessagetoClient(int clientId, int error_status, char* msg)
{
    char ans_name[100];
    snprintf(ans_name, 100, "ans%lu", (unsigned long)clientId);
    int answer_fd = open(ans_name, O_WRONLY);

    char aux[8];
    snprintf(aux, 8, "%d", error_status);
    write(answer_fd, aux, 8);

    if(msg != NULL)
    {
        write(answer_fd, msg, 1000);
    }

    close(answer_fd);
}