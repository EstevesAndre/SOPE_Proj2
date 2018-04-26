typedef struct
{
        int client_id;
        int n_seats;
        int seats[];
        int error_status;
} request;