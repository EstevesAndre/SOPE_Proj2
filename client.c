#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "client.h"
#include "constants.h"

FILE* requests_fd, *answer_fd;
char ans_name[100];

int main(int argc, char *argv[])
{
        signal(SIGALRM, timeout);
        int n_seats, open_time;
        char* seats_list = malloc(100* sizeof(char));

        client_argchk(argc, argv, &open_time, &n_seats, seats_list);

        alarm(open_time);

        int fd = open("requests", O_WRONLY | O_NONBLOCK);
        requests_fd = fdopen(fd, "w");

        snprintf(ans_name, 100, "ans%lu", (unsigned long)getpid());
        mkfifo(ans_name, 0660);
        fd = open(ans_name, O_RDONLY | O_NONBLOCK);
        answer_fd = fdopen(fd, "r");

        sendMessage(requests_fd, n_seats, seats_list);
        
        char* reserve = malloc(100 * sizeof(char));

        fgets(reserve, 100, answer_fd);

        long return_status = strtol(reserve, NULL, 10);

        if(return_status == REQ_SUCCESSFUL)
        {
                fgets(reserve, 100, answer_fd);
                printReserve(reserve);
                remove(ans_name);
                return 0;
        }
        else
        {
                printError(return_status);
                remove(ans_name);
                return return_status;
        }

}

void client_argchk(int argc, char* argv[], int* open_time, int* n_seats, char* seats_list)
{
        if(argc == 1)
        {
                printf("Usage: %s  <time_out> <num_wanted_seats> <pref_seat_list>\n", argv[0]);
                exit(0);
        }
        if(argc != 4)
        {
                printf("Invalid number of arguments: %s\n", argv[0]);
                exit(1);
        }
        else
        {
                *open_time = strtol(argv[1], NULL, 10);

                if(*open_time <= 0)
                {
                        printf("Invalid argument: %s\n", argv[1]);
                        exit(1);
                }

                *n_seats = strtol(argv[2], NULL, 10);

                if(*n_seats <= 0)
                {
                        printf("Invalid argument: %s\n", argv[2]);
                        exit(1);
                }

                strcpy(seats_list, argv[3]);
        }
}

void timeout(int signo)
{
        //fclose(requests_fd);
        //fclose(answer_fd);
        remove(ans_name);
        printf("Timed out - no response from server.\n");
        exit(2);
}

void sendMessage(FILE* fd, int n_seats, char* seats_list)
{
        char message[200];
        snprintf(message, 200, "%d %d %s", getpid(), n_seats, seats_list);
        fprintf(fd, "%s", message);
}

void printReserve(char* reserve)
{
        char * delim_1 = strtok(reserve, " ");
        int n_seats = strtol(reserve, NULL, 10);

        printf("Number of seats reserved: %d\n", n_seats);
        printf("Seats reserved: %s", delim_1);
}

void printError(int return_status)
{
        printf("Failed to reserve seats: ");
        switch(return_status)
        {
        case REQ_ERR_OVER_MAX_SEATS:
        {
                printf("Exceeded maximum number of seats per request.\n");
                return;
        }

        case REQ_ERR_UNDER_SEAT_ID:
        {
                printf("Invalid number of seat identifiers.\n");
                return;
        }

        case REQ_ERR_SEAT_ID_INV:
        {
                printf("Invalid seat identifiers.\n");
                return;
        }

        case REQ_ERR_PARAM_OTHER:
        {
                printf("Input error.\n");
                return;
        }

        case REQ_ERR_UNNAV_SEAT:
        {
                printf("Desired seats unnavailable.\n");
                return;
        }

        case REQ_ERR_ROOM_FULL:
        {
                printf("Room is Full.\n");
                return;
        }

        default:
        {
                printf("Unknown error.\n");
                return;
        }
        }
}
