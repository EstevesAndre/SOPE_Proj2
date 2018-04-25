#include <stdio.h> 
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> 
#include "server.h"
#include "constants.h"

int main(int argc, char *argv[])
{
time_t start = time(NULL);
int n_seats, n_offices, open_time;

argchk(argc, argv, &n_seats, &n_offices, &open_time);

while(1) {
    int seats[n_seats];
    initSeats(seats, n_seats);

    int requests = createRequestFifo();

    if (time(NULL) > start + open_time) {
        //timeout
        break;
    }
}
}

void argchk(int argc, char* argv[], int* n_seats, int* n_offices, int* open_time)
{
    if(argc == 1)
    {
        printf("Usage: %s <num_room_seats> <num_ticket_offices> <open_time>\n", argv[0]);
        exit(0);
    }
    if(argc != 4)
    {
        printf("Invalid number of arguments: &s\n", argv[0]);
        exit(1);
    }
    else
    {
        *n_seats = strtol(argv[1], NULL, 10);

        if(*n_seats <= 0 || *n_seats > MAX_ROOM_SEATS)
        {
            printf("Invalid argument: &s\n", argv[1]);
            exit(1);
        } 

        *n_offices = strtol(argv[2], NULL, 10);

        if(*n_seats <= 0)
        {
            printf("Invalid argument: &s\n", argv[2]);
            exit(1);
        } 

        *open_time = strtol(argv[2], NULL, 10);

        if(*open_time <= 0)
        {
            printf("Invalid argument: &s\n", argv[3]);
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