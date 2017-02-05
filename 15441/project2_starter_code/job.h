#ifndef _JOB_H_
#define _JOB_H_

#include <sys/types.h>
#include "sha.h"
#include "bt_parse.h"
#include "queue.h"

#define PACKET_LEN 1500
#define HEADER_LEN 16
#define DATA_LEN (PACKET_LEN - HEADER_LEN)
#define BUF_SIZE 60
#define MAX_CHUNK 74

#define PKT_WHOHAS  0
#define PKT_IHAVE   1
#define PKT_GET     2
#define PKT_DATA    3
#define PKT_ACK     4
#define PKT_DENIED  5


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
    char data[DATA_LEN] ;
} packet_t;

typedef struct chunk_s {
    int id;
    uint8_t hash[SHA1_HASH_SIZE];
    char *data;
    int cur_size;
    int num_p;
    bt_peer_t *pvd;
} chunk_t;

typedef struct job_s {
    int num_chunks;
    int num_need;
    int num_living;
    chunk_t* chunks;
    char get_chunk_file[BT_FILENAME_LEN];
} job_t;

//job method
int init_job(char* chunkFile, char* output_file, job_t* job);

//logic method
queue_t* make_whohas(job_t* job);
void send_whohas(packet_t* p);
//packet method
packet_t* init_packet(int type, short packet_len, u_int seq,
        u_int ack, char* data);
void destroy_packet(packet_t*);
packet_t* parse_request(char*, size_t);
void send_packet(packet_t* pkt, struct sockaddr* to);

void print_pkt(packet_t*);
void netToHost(packet_t*);
void hostToNet(packet_t*);
#endif
