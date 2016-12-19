#ifndef _JOB_H_
#define _JOB_H_

#define PACKET_LEN 1500
#define HEADER_LEN 20
#define DATA_LEN (PACKET_LEN - HEADER_LEN)

#define CHUNK_HASH_LEN 20

typedef struct header_s {
    short magic_num;
    char version;
    char type;
    short header_len;
    short packet_len;
    u_int seq_num;
    u_int ack_num;
} header_t;

typedef struct packet_s {
    header_t header;
    char[DATA_LEN] data;
} packet_t;

#endif