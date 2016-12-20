#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "job.h"

packet_t* init_packet(int type, short packet_len, u_int seq,
        u_int ack, char* data) {
    packet_t * pkt = (packet_t*)malloc(sizeof(packet_t));
    if (pkt == NULL) {
        return NULL;
    }
    pkt->header.magic_num = 15441;
    pkt->header.version = 1;
    pkt->header.type = type;
    pkt->header.header_len = HEADER_LEN;
    pkt->header.packet_len = packet_len;
    pkt->header.seq_num = seq;
    pkt->header.ack_num = ack;
    memcpy(pkt->data, data, packet_len-HEADER_LEN);
    return pkt;
}

void destroy_packet(packet_t* pkt) {
    free(pkt);
}

packet_t* parse_request(char* data, size_t length) {
    packet_t* pkt = (packet_t*)data;
    netToHost(pkt);
    return pkt;
}

//helping
void netToHost(packet_t* pkt) {
    pkt->header.magic_num = ntohs(pkt->header.magic_num);
    pkt->header.header_len = ntohs(pkt->header.header_len);
    pkt->header.packet_len = ntohs(pkt->header.packet_len);
    pkt->header.seq_num = ntohl(pkt->header.seq_num);
    pkt->header.ack_num = ntohl(pkt->header.ack_num);
}

void hostToNet(packet_t* pkt) {
    pkt->header.magic_num = htons(pkt->header.magic_num);
    pkt->header.header_len = htons(pkt->header.header_len);
    pkt->header.packet_len = htons(pkt->header.packet_len);
    pkt->header.seq_num = htonl(pkt->header.seq_num);
    pkt->header.ack_num = htonl(pkt->header.ack_num);
}

void print_pkt(packet_t* pkt) {
    header_t* hdr = &pkt->header;
    fprintf(stderr, ">>>>>>>>>START<<<<<<<<<<<<<\n");
    fprintf(stderr, "magicnum:\t\t%d\n", hdr->magic_num);
    fprintf(stderr, "version:\t\t%d\n", hdr->version);
    fprintf(stderr, "packet_type:\t\t%d\n", hdr->type);
    fprintf(stderr, "header_len:\t\t%d\n", hdr->header_len);
    fprintf(stderr, "packet_len:\t\t%d\n", hdr->packet_len);
    fprintf(stderr, "seq_num:\t\t%d\n", hdr->seq_num);
    fprintf(stderr, "ack_num:\t\t%d\n", hdr->ack_num);
}
