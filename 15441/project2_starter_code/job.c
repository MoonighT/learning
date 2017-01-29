#include "job.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chunk.h"
#include "queue.h"

extern bt_config_t config;

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

int init_job(char* chunkFile, char* output_file, job_t* job/*in out*/) {
    //read chunk file
    FILE* file = fopen(chunkFile, "r");
    if(file == NULL) {
        fprintf(stderr, "chunk file cannot open %s", chunkFile);
        return -1;
    }
    //get chunk nums
    char read_buffer[BUF_SIZE];
    char hash_buffer[SHA1_HASH_SIZE * 2];
    int num_line = 0;
    int i = 0;
    while(fgets(read_buffer, BUF_SIZE, file)) {
        num_line++;
    }
    job->num_chunks = num_line;
    job->num_need = num_line;
    job->num_living = 0;
    job->chunks = malloc(sizeof(chunk_t) * num_line);
    fseek(file, 0, SEEK_SET);
    while(fgets(read_buffer, BUF_SIZE, file)) {
        sscanf(read_buffer, "%d %s", &job->chunks[i].id, hash_buffer);
        hex2binary(hash_buffer, SHA1_HASH_SIZE*2, job->chunks[i].hash);
        job->chunks[i].pvd = NULL;
        job->chunks[i].num_p= 0;
        job->chunks[i].cur_size= 0;
        job->chunks[i].data= malloc(sizeof(char) * 512 * 1024);
        i++;
        memset(read_buffer, 0, BUF_SIZE);
        memset(hash_buffer, 0, SHA1_HASH_SIZE*2);
    }
    fclose(file);
    strcpy(config.output_file, output_file);
    strcpy(job->get_chunk_file, chunkFile);
    config.output_file[strlen(output_file)] = '\0';
    job->get_chunk_file[strlen(chunkFile)] = '\0';
    return 0;
}

queue_t* make_whohas(job_t* job) {
    short pkt_len = 0;
    char* data[DATA_LEN];
    if (job->num_chunks == 0) {
        return NULL;
    }
    queue_t* q = queue_init();
    if(job->num_chunks > MAX_CHUNK) {
        //many pkt
        int i;
        int n = job->num_chunks / MAX_CHUNK;
        int len;
        for(i = 0; i <= n; i++) {
            len = MAX_CHUNK;
            if(i==n) {
                len = job->num_chunks - n * MAX_CHUNK;
            }
            pkt_len =  HEADER_LEN + 4 + len * SHA1_HASH_SIZE;
            make_whohas_data(len, job->chunks+i*MAX_CHUNK, data);
            packet_t* pkt = init_packet(PKT_WHOHAS, pkt_len, 0, 0, data);
            enqueue(q, pkt);
        }
    } else {
        //1 pkt
        pkt_len =  HEADER_LEN + 4 + job->num_chunks * SHA1_HASH_SIZE;
        make_whohas_data(job->num_chunks, job->chunks, data);
        packet_t* pkt = init_packet(PKT_WHOHAS, pkt_len, 0, 0, data);
        enqueue(q, pkt);
    }
    return q;
}

void make_whohas_data(int num_chunks, chunk_t* chunks, char* data) {
    memset(data, 0, 4);
    int i = 0;
    int j = 0;
    char* ptr = data+4;
    for (i =0; i < num_chunks; ++i) {
        if(chunks[i].cur_size == 512*1024)
            continue;
        memcpy(ptr + j * SHA1_HASH_SIZE, chunks[i].hash, SHA1_HASH_SIZE);
        j++;
    }
    data[0] = j;
}

void send_whohas(packet_t* pkt) {
    bt_peer_t* peer = config.peers;
    while(peer != NULL) {

    }
}
