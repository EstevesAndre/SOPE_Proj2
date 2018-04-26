typedef struct
{
        int client_id;
        int n_seats;
        int seats[];
        int array_cnt = 0;
        int error_status;
} request;