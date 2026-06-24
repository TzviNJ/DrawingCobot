/*###############################
# @file: err_codes.h
# @breif: error codes
# Date Created: 11.1.26
# Version 0.5.4
###############################*/

#ifndef ERR_CODES_H
#define ERR_CODES_H

#include <stdio.h>

typedef enum err_codes {
    OK = 0,
    
    ERR_BAD_ARGS,
    ERR_MEMORY,
    ERR_FILE,
    ERR_FORK,
    ERR_EXEC,
    ERR_PIPE,
    ERR_SIG,

    ERR_UNKNOWN
} Err_Codes;

static void print_errcode(int err_code, FILE* fd) {
    switch(err_code) {
        case OK:
            fprintf(fd, "OK");
            break;
        case ERR_BAD_ARGS:
            fprintf(fd ,"ERR_BAD_ARGS");
            break;
        case ERR_MEMORY:
            fprintf(fd, "ERR_MEMORY");
            break;
        case ERR_FILE:
            fprintf(fd, "ERR_FILE");
            break;
        case ERR_FORK:
            fprintf(fd, "ERR_FORK");
            break;
        case ERR_EXEC:
            fprintf(fd, "ERR_EXEC");
            break;
        case ERR_SIG:
            fprintf(fd, "ERR_SIG");
            break;
        default:
            fprintf(fd, "UNKNOWN ERROR");
            break;
    }
}

#endif /*ERR_CODES_H*/
