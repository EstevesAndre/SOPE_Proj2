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

int descrit_slog = -1;
int descrit_sbook = -1;

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

        char *server_logfile = "slog.txt";
        descrit_slog = open(server_logfile, O_WRONLY | O_APPEND | O_CREAT, 0644);

        char *server_booking = "sbook.txt";
        descrit_sbook = open(server_booking, O_WRONLY | O_APPEND | O_CREAT, 0644);

        int i;
        for (i=1; i<=n_offices; i++)
        {
                if(descrit_slog != -1)
                {
                        char numb[3];
                        sprintf(numb,"%02d", i);

                        char* ch = "-OPEN";
                        char* result = malloc(strlen(ch) + strlen(numb) + 2);

                        strcpy(result,numb);
                        strcat(result,ch);
                        strcat(result, "\n");

                        write(descrit_slog,result,strlen(result));
                }
                pthread_create(&offices[i-1], NULL, office_f, seats);
        }

        FILE* requests = createRequestFifo();
        request r;

        while(1) {
                char* info = malloc(100 * sizeof(char));
                char* k = fgets(info, 100, requests);
                if(k != NULL)
                {
                        parseRequest(&r, info, n_seats);
                        while(buffer != NULL)
                        {
                                if (time(NULL) > start + open_time)
                                {
                                        printf("Timeout. Server closing.\n");
                                        fclose(requests);
                                        remove("requests");
                                        if(descrit_slog != -1)
                                        {

                                                for (i=1; i<=n_offices; i++)
                                                {
                                                        char numb[3]; // 2 caracteres
                                                        snprintf(numb, sizeof numb, "%02d", i);

                                                        char* ch = "-CLOSED";
                                                        char* result = malloc(strlen(ch) + strlen(numb) + 2);

                                                        strcpy(result,numb);
                                                        strcat(result,ch);
                                                        strcat(result, "\n");

                                                        write(descrit_slog,result,strlen(result));
                                                }
                                                char * end = "SERVER CLOSED\n";
                                                write(descrit_slog,end,strlen(end));
                                                close(descrit_slog);
                                        }
                                        if(descrit_sbook != -1)
                                        {
                                                close(descrit_sbook);
                                        }
                                        exit(0);
                                }
                        }
                        buffer = &r;
                }

                if (time(NULL) > start + open_time)
                {
                        printf("Timeout. Server closing.\n");
                        fclose(requests);
                        remove("requests");

                        if(descrit_slog != -1)
                        {
                                for (i=1; i<=n_offices; i++)
                                {
                                        char numb[3];
                                        snprintf(numb, sizeof numb, "%02d", i);

                                        char* ch = "-CLOSED";
                                        char* result = malloc(strlen(ch) + strlen(numb) + 2);

                                        strcpy(result,numb);
                                        strcat(result,ch);
                                        strcat(result, "\n");

                                        write(descrit_slog,result,strlen(result));
                                }
                                char * end = "SERVER CLOSED\n";
                                write(descrit_slog,end,strlen(end));
                                close(descrit_slog);
                        }
                        if(descrit_sbook != -1)
                        {
                                close(descrit_sbook);
                        }
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
                seats[i] = 0;
        }
}

FILE* createRequestFifo()
{
        mkfifo("requests",0660);
        int fd = open("requests", O_RDONLY | O_NONBLOCK);
        FILE* f = fdopen(fd, "r");
        if(f == NULL)
        {
                printf("Failed to create requests fifo\n");
                if(descrit_sbook != -1)
                        close(descrit_sbook);
                if(descrit_slog != -1)
                        close(descrit_slog);
                exit(2);
        }
        return f;
}

void parseRequest(request* r, char* info, int n_seats)
{
        r->error_status = 0;

        r->client_id = strtol(info, NULL, 10);
        if(r->client_id == 0)
        {
                r->error_status = REQ_ERR_PARAM_OTHER;
                return;
        }

        char* pos = info;

        while(*pos != ' ')
        {
                pos++;
        }
        pos++;

        r->n_seats = strtol(pos, NULL, 10);
        if(r->n_seats == 0)
        {
                r->error_status = REQ_ERR_PARAM_OTHER;
                return;
        }

        while(*pos != ' ')
        {
                pos++;
        }
        pos++;

        r->seats = malloc(sizeof(int) * MAX_CLI_SEATS);
        r->array_cnt = 0;
        int c, bytesread;
        while (sscanf(pos, "%d%n", &c, &bytesread) > 0) 
        {
                *(r->seats + (r->array_cnt++)) = c;
                pos += bytesread;
        }

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
                sendMessagetoClient(r, r->error_status, NULL, NULL);
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
                        sendMessagetoClient(r, REQ_ERR_UNNAV_SEAT, NULL,NULL);
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
        char nrseat[6];
        char message2log[1000];

        snprintf(message, 1000, "%d", r->n_seats);
        strcpy(message2log,"");
        int i;
        for(i = 0; i < reserve_cnt; i++)
        {
                snprintf(aux, 8, " %d",reserved_seats[i]);
                strcat(message, aux);

                snprintf(nrseat, 6, " %04d",reserved_seats[i]);
                strcat(message2log,nrseat);

                if(descrit_sbook != -1)
                {
                        char * sbooklog = malloc(strlen(nrseat)+ 2);
                        strcpy(sbooklog,nrseat);
                        strcat(sbooklog,"\n");
                        write(descrit_sbook,sbooklog,strlen(sbooklog));
                }

        }

        strcat(message, "\n");
        strcat(message2log, "\n");
        sendMessagetoClient(r, REQ_SUCCESSFUL, message, message2log);
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

void sendMessagetoClient(request* r, int error_status, char* msg, char *msg2log)
{
        char ans_name[100];
        snprintf(ans_name, 100, "ans%lu", (unsigned long)r->client_id);

        int fd = open(ans_name, O_WRONLY | O_NONBLOCK);
        FILE* answer_fd = fdopen(fd, "w");

        char aux[8];
        snprintf(aux, 8, "%d", error_status);
        setbuf(answer_fd, NULL);
        fprintf(answer_fd, "%s\n", aux);

        if(msg != NULL)
        {
                fprintf(answer_fd, "%s", msg);
        }

        if(descrit_slog != -1)
        {
                char* store_nr = "99";
                char* ch = "-";
                char* twoPoints = ":";

                char idClient[WIDTH_PID+1];
                snprintf(idClient, WIDTH_PID+1, "%lu", (unsigned long)r->client_id); // 5 digits

                char nr_Seats[3];
                snprintf(nr_Seats, 3, "%02d", r->n_seats); // 2 digits

                int tmh = 4;
                if(msg != NULL)
                        tmh = strlen(msg2log);

                char* result = malloc(3*strlen(ch) + strlen(store_nr) + strlen(idClient) + strlen(nr_Seats) +
                                      strlen(twoPoints) + 26 + tmh + 2);

                strcpy(result,store_nr);
                strcat(result,ch);
                strcat(result,idClient);
                strcat(result,ch);
                strcat(result,nr_Seats);
                strcat(result,twoPoints);

                int i;

                char* space = " ";
                for(i = 0; i < r->n_seats; i++)
                {
                        strcat(result,space);
                        char number_Seat[WIDTH_SEAT + 1];
                        snprintf(number_Seat, WIDTH_SEAT + 1, "%04d", *(r->seats + i));
                        strcat(result, number_Seat);
                }
                for(i = 0; i < WIDTH_XXNN - r->n_seats; i++)
                        strcat(result,"     ");

                strcat(result,space);
                strcat(result,ch);

                if(msg == NULL)
                {
                        switch (error_status) {
                        case REQ_ERR_OVER_MAX_SEATS:
                                strcat(result," MAX\n");
                                break;
                        case REQ_ERR_UNDER_SEAT_ID:
                                strcat(result," NST\n");
                                break;
                        case REQ_ERR_SEAT_ID_INV:
                                strcat(result," IID\n");
                                break;
                        case REQ_ERR_PARAM_OTHER:
                                strcat(result," ERR\n");
                                break;
                        case REQ_ERR_UNNAV_SEAT:
                                strcat(result," NAV\n");
                                break;
                        case REQ_ERR_ROOM_FULL:
                                strcat(result," FUL\n");
                                break;
                        default:
                                break;
                        }
                }
                else
                {
                        strcat(result,msg2log);
                }

                write(descrit_slog,result,strlen(result));
        }
}
