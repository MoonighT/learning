/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2.
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bt_parse.h"
#include "debug.h"
#include "input_buffer.h"
#include "job.h"
#include "processor.h"
#include "queue.h"
#include "spiffy.h"

void peer_run(bt_config_t *config);

bt_config_t config;
job_t globaljob;

int main(int argc, char **argv) {

    bt_init(&config, argc, argv);

    DPRINTF(DEBUG_INIT, "peer.c main beginning\n");

#ifdef TESTING
    config.identity = 1; // your group number here
    strcpy(config.chunk_file, "chunkfile");
    strcpy(config.has_chunk_file, "haschunks");
#endif

    bt_parse_command_line(&config);

#ifdef DEBUG
    if (debug & DEBUG_INIT) {
        bt_dump_config(&config);
    }
#endif

    bt_dump_config(&config);
    peer_run(&config);
    return 0;
}


void process_inbound_udp(int sock) {
#define BUFLEN 1500
    printf("process inbound udp\n");
    struct sockaddr_in from;
    socklen_t fromlen;
    char buf[BUFLEN];

    fromlen = sizeof(from);
    spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from,
            &fromlen);

    printf("PROCESS_INBOUND_UDP SKELETON -- replace!\n"
            "Incoming message from %s:%d\n%s\n\n",
            inet_ntoa(from.sin_addr),
            ntohs(from.sin_port),
            buf);
    packet_t* request = parse_request(buf, BUFLEN);
    if(request == NULL) {
        fprintf(stderr, "request pkt is null\n");
        return;
    }
    //process logic
    packet_t* reply = process_message(request);
    if(reply != NULL) {
        send_packet(reply, (struct sockaddr*)&from);
    } else {
        fprintf(stderr, "reply pkt is null\n");
    }
    //free packet
    destroy_packet(request);
    destroy_packet(reply);
}

void process_get(char *chunkfile, char *outputfile) {
    printf("PROCESS GET SKELETON CODE CALLED.  Fill me in!  (%s, %s)\n",
            chunkfile, outputfile);
    //construct whohas packet
    //make_whohas_packet
    init_job(chunkfile, outputfile, &globaljob);
    queue_t *whohas_packets = make_whohas(&globaljob);
    packet_t* cur_pkt = NULL;
    while((cur_pkt = (packet_t*)dequeue(whohas_packets)) != NULL) {
        send_whohas(cur_pkt);
//        destroy_packet(cur_pkt);
    }
    printf("finish process get \n");
}

void handle_user_input(char *line, void *cbdata) {
    char chunkf[128], outf[128];

    printf("handle user input \n");
    bzero(chunkf, sizeof(chunkf));
    bzero(outf, sizeof(outf));

    if (sscanf(line, "GET %120s %120s", chunkf, outf)) {
        if (strlen(outf) > 0) {
            process_get(chunkf, outf);
        }
    }
}


void peer_run(bt_config_t *config) {
    int sock;
    struct sockaddr_in myaddr;
    fd_set readfds;
    struct user_iobuf *userbuf;

    if ((userbuf = create_userbuf()) == NULL) {
        perror("peer_run could not allocate userbuf");
        exit(-1);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("peer_run could not create socket");
        exit(-1);
    }

    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(config->myport);

    if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        perror("peer_run could not bind socket");
        exit(-1);
    }
    config->sock = sock;
    spiffy_init(config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));
    //init has chunk
    //

    printf("finish init \n");
    while (1) {
        int nfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        nfds = select(sock+1, &readfds, NULL, NULL, NULL);

        if (nfds > 0) {
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                process_user_input(STDIN_FILENO, userbuf, handle_user_input,
                        "Currently unused");
            }

            if (FD_ISSET(sock, &readfds)) {
                process_inbound_udp(sock);
            }
        }
    }
}
