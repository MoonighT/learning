#include "job.h"


packet_t* init_packet(int type, short packet_len, u_int seq,
        u_int ack, char* data) {
    packet_t * pkt = (packet_t*)malloc(sizeof(packet_t));
    pkt->header.magic_num = 15441;
    pkt->header.version = 1;
    pkt->header.type = type;

}

void destroy_packet(packet_t*);
packet_t* parse_request(char*, size_t);

