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

int requests_fd, answer_fd;

int main(int argc, char *argv[])
{
signal(SIGALRM, timeout);
int n_seats, open_time;
char* seats_list;

client_argchk(argc, argv, &open_time, &n_seats, seats_list);
alarm(open_time);

requests_fd = open("requests", O_WRONLY);

char ans_name[100];
snprintf(ans_name, 100, "ans%lu", (unsigned long)getpid());
mkfifo(ans_name, 0660);
answer_fd = open(ans_name, O_RDONLY);

sendMessage(requests_fd, n_seats, seats_list);

}

void client_argchk(int argc, char* argv[], int* open_time, int* n_seats, char* seats_list)
{
    if(argc == 1)
    {
        printf("Usage: %s  <time_out> <num_wanted_seats> <pref_seat_list\n", argv[0]);
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

        seats_list = malloc(100 *  sizeof(char));
        strcpy(seats_list, argv[3]); 
    }
}

void timeout(int signo)
{
    close(requests_fd);
    close(answer_fd);
    exit(2);
}

void sendMessage(int fd, int n_seats, char* seats_list)
{
    char message[200];
    snprintf(message, 200, "%d %d %s", getpid(), n_seats, seats_list); 
    
}