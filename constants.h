#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4

#define REQ_SUCCESSFUL 0
#define REQ_ERR_OVER_MAX_SEATS -1
#define REQ_ERR_UNDER_SEAT_ID -2
#define REQ_ERR_SEAT_ID_INV -3
#define REQ_ERR_PARAM_OTHER -4
#define REQ_ERR_UNNAV_SEAT -5
#define REQ_ERR_ROOM_FULL -6

#define Seat int
#define DELAY() sleep(1)

#endif
