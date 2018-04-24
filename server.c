#include <time.h>
#include "server.h"
#include "constants.h"

int main(int argc, char *argv[])
{
time_t start = time(NULL);
int n_seats, n_offices, open_time;

argchk(argc, argv, &n_seats, &n_offices, &open_time);

while(1) {

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
        printf("Usage: %s <num_room_seats> <num_ticket_offices> <open_time>", argv[0]);
        exit(0);
    }
    if(argc != 4)
    {
        printf("Invalid number of arguments: &s", argv[0]);
        exit(1);
    }
    else
    {
        //parser - strtol
    }
}