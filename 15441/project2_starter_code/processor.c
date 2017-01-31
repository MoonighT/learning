#include "processor.h"
#include <stdio.h>

packet_t* process_message(packet_t* request) {
    if(request == NULL ) {
        fprintf(stderr, "process message request is NULL\n");
        return NULL;
    }
    header_t hder = request->header;
    if(hder.magic_num != 15441 || hder.version != 1) {
        fprintf(stderr, "process message request is invalid\n");
        return NULL;
    }
    char type = hder.type;
    fprintf(stdout, "process message type is %d \n", type);
    return NULL;
}

