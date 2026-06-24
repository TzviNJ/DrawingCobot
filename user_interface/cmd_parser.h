/*###############################
# @file: cmd_parser.h
# @breif: Parse user commands
# Date Created: 11.1.26
# Version 1.2.0
###############################*/

#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include "err_codes.h"
#include "config.h"
#include "launch.h"
#include <string.h>

typedef enum user_cmd_type {
    CMD_NONE = 0,
    CMD_EXIT,
    CMD_HELP,
    CMD_LAUNCH_POL,
    CMD_LAUNCH_CART,
    CMD_LAUNCH_GRIP,
    CMD_OPEN_GRIP,
    CMD_CLOSE_GRIP,
    CMD_RUN,
    CMD_RUN_STOP,
    CMD_LAUNCH_TEST,
    CMD_EMERGANCY_STOP,
    CMD_UNKNOWN
} User_Cmd_Type;

/*******************************************************
 * @struct user_cmd
 * @brief Holds a parsed user command.
*******************************************************/
typedef struct user_cmd {
    User_Cmd_Type type;
    int progmode;
    char imgname[MAX_CMD_LEN];
} User_Cmd;

/*******************************************************
 * @name parse_cmd
 * @brief Parse cmd prompt input.
 * @param cmd_text_original Raw text from user
 * @param target_cmd_struct Target struct for parsed cmd
 * @retval err_code (enum defined in err_codes.h)
*******************************************************/
static int parse_cmd(char *cmd_text_original, User_Cmd *target_cmd_struct) {
    Err_Codes err_code = OK;
    int word_num = 0;
    char cmd_text[MAX_CMD_LEN+1];
    char *word = NULL;
    char **nptr = NULL;
    /* Check args */
    if ((cmd_text == NULL) || (target_cmd_struct == NULL))
        return ERR_BAD_ARGS;
    /* Copy original string to local copy */
    strcpy(cmd_text, cmd_text_original);
    /* Init */
    target_cmd_struct->type = CMD_UNKNOWN;
    target_cmd_struct->progmode = -1;
    (target_cmd_struct->imgname)[0] = '\0';
    /* Remove newline */
    if ((strlen(cmd_text) > 0) && (cmd_text[strlen(cmd_text)-1] == '\n'))
        cmd_text[strlen(cmd_text)-1] = '\0';
    /* Parse individual words */
    word = strtok(cmd_text, " ");
    while (word != NULL) {
        if ((word_num == 0) && (strcmp(word, EXIT_CMD_STR) == 0)) {
            target_cmd_struct->type = CMD_EXIT;
        }
        else if ((word_num == 0) && (strcmp(word, HELP_CMD_STR) == 0)) {
            target_cmd_struct->type = CMD_HELP;           
        }
        else if ((word_num == 0) && (strcmp(word, LAUNCH_CART_STR) == 0)) {
            target_cmd_struct->type = CMD_LAUNCH_CART;           
        }
        else if ((word_num == 0) && (strcmp(word, LAUNCH_POL_STR) == 0)) {
            target_cmd_struct->type = CMD_LAUNCH_POL;           
        }
        else if ((word_num == 0) && (strcmp(word, LAUNCH_GRIP_STR) == 0)) {
            target_cmd_struct->type = CMD_LAUNCH_GRIP;           
        }
        else if ((word_num == 0) && (strcmp(word, OPEN_GRIP_STR) == 0)) {
            target_cmd_struct->type = CMD_OPEN_GRIP;           
        }
        else if ((word_num == 0) && (strcmp(word, CLOSE_GRIP_STR) == 0)) {
            target_cmd_struct->type = CMD_CLOSE_GRIP;           
        }
        else if ((word_num == 0) && (strcmp(word, LAUNCH_TEST_STR) == 0)) {
            target_cmd_struct->type = CMD_LAUNCH_TEST;           
        }
        else if ((word_num == 0) && (strcmp(word, STOP_PROG_STR) == 0)) {
            target_cmd_struct->type = CMD_RUN_STOP;           
        }
        else if ((word_num == 0) && (strcmp(word, EMGNCY_STOP_STR) == 0)) {
            target_cmd_struct->type = CMD_EMERGANCY_STOP;           
        }
        else if ((word_num == 0) && (strcmp(word, LAUNCH_PROG_STR) == 0)) {
            target_cmd_struct->type = CMD_RUN;           
        }
        else if ((word_num == 1) && target_cmd_struct->type == CMD_RUN) {
            target_cmd_struct->progmode = (int) strtol(word, nptr, DECIMAL);
        }
        else if ((word_num == 2) && target_cmd_struct->type == CMD_RUN) {
            strcpy(target_cmd_struct->imgname, word);
        }
        word = strtok(NULL, " ");
        word_num++;
    }
    return err_code;
}

#endif /*CMD_PARSER_H*/