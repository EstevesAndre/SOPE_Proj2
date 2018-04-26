void client_argchk(int argc, char* argv[], int* open_time, int* n_seats, char* seats_list);
void timeout(int signo);
void sendMessage(int fd, int n_seats, char* seats_list);