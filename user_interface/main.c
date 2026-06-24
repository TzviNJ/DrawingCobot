/*###################################
# @file: main.c
# @breif: main
# Date Created: 11.1.26
# Version 1.2.0
###################################*/

#include "config.h"
#include "err_codes.h"
#include "cmd_parser.h" // + "launch.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void cleanup();

/*****************************************************
 * @name main
 * @brief Main
 * @remarks Low level user interface for controlling
 *      launches and program.
 * @param argc
 * @param argv
 * @retval err_code (enum defined in err_codes.h)
*****************************************************/
int main(int argc, char *argv[]) {
    /* Register Signal Handlers */
    struct sigaction sigact;
    sigact.sa_handler = cleanup;
    sigemptyset(&sigact.sa_mask);
    if (sigaction(SIGINT, &sigact, NULL) == -1)
        ERR_EXIT("Signal Registration", ERR_SIG);    
    if (sigaction(SIGTERM, &sigact, NULL) == -1)
        ERR_EXIT("Signal Registration", ERR_SIG);

    Err_Codes err_code = OK;
    Launch launch;
    User_Cmd user_cmd;
    char cmd_text[MAX_CMD_LEN+1];
    int program_running = FALSE;
    int test_running = FALSE;
    /* Initialization */
    user_cmd.type = CMD_NONE;
    launch.type = NO_LAUNCH;
    cmd_text[MAX_CMD_LEN] = '\0';
    /* Prompt */
    printf("Write '%s' for help, '%s' to exit.\n", HELP_CMD_STR, EXIT_CMD_STR);
    /* User Input */
    while (user_cmd.type != CMD_EXIT) {
        print_launch_type(&launch, stdout);
        printf(": ");
        fgets(cmd_text, (MAX_CMD_LEN+1), stdin);
        err_code = parse_cmd(cmd_text, &user_cmd);
        if (err_code != OK) {
            cleanup();
            ERR_EXIT("parse err", ERR_BAD_ARGS);
        }
        switch (user_cmd.type) {
            case CMD_HELP:
                printf("HELP: Write '%s' for help.\n", HELP_CMD_STR);
                printf("HELP: Write '%s' to exit.\n", EXIT_CMD_STR);
                printf("HELP: Write '%s' to launch cart.\n", LAUNCH_CART_STR);
                printf("HELP: Write '%s' to launch pol.\n", LAUNCH_POL_STR);
                printf("HELP: Write '%s' to launch gripper.\n", LAUNCH_GRIP_STR);
                printf("HELP: Write '%s' to open gripper. GRIP MODE must be launched first.\n", OPEN_GRIP_STR);
                printf("HELP: Write '%s' to close gripper. GRIP MODE must be launched first.\n", CLOSE_GRIP_STR);
                printf("HELP: Write '%s PROGMODE IMG' to start program.\n", LAUNCH_PROG_STR);
                printf("HELP: Write '%s' to stop program.\n", STOP_PROG_STR);
                printf("HELP: Write '%s' to toggle launch test sequence.\n", LAUNCH_TEST_STR);
                printf("HELP: Write '%s' to EMERGANCY STOP program and gripper.\n", EMGNCY_STOP_STR);
                break;
            case CMD_EXIT:
                printf("Exiting...\n");
                cleanup_launches();
                break;
            case CMD_LAUNCH_CART:
                if (program_running == TRUE) {
                    printf("Killing program...\n");
                    err_code = kill_launch(PROG_LAUNCH, DEFAULT_KILL_TIMEOUT);
                    program_running = FALSE;
                }
                if (launch.type == POL_LAUNCH) {
                    printf("Killing polar mode launch...\n");
                    err_code = kill_launch(POL_LAUNCH, DEFAULT_KILL_TIMEOUT);
                }
                if (launch.type == GRIP_LAUNCH) {
                    printf("Killing gripper mode launch...\n");
                    err_code = kill_launch(GRIP_LAUNCH, DEFAULT_KILL_TIMEOUT);
                }
                launch.type = CART_LAUNCH;
                do {
                    printf("Launching robot in cartesian mode...\n");
                    err_code = launch_seq(CART_LAUNCH, 0, NULL, 1.0);
                } while(err_code != OK);
                break;
            case CMD_LAUNCH_POL:
                if (program_running == TRUE) {
                    printf("Killing program...\n");
                    err_code = kill_launch(PROG_LAUNCH, DEFAULT_KILL_TIMEOUT);
                    program_running = FALSE;
                }
                if (launch.type == CART_LAUNCH) {
                    printf("Killing cartesian mode launch...\n");
                    err_code = kill_launch(CART_LAUNCH, DEFAULT_KILL_TIMEOUT);
                }
                if (launch.type == GRIP_LAUNCH) {
                    printf("Killing gripper mode launch...\n");
                    err_code = kill_launch(GRIP_LAUNCH, DEFAULT_KILL_TIMEOUT);
                }
                launch.type = POL_LAUNCH;
                do {
                    printf("Launching robot in polar mode...\n");
                    err_code = launch_seq(POL_LAUNCH, 0, NULL, 1);
                } while(err_code != OK);
                break;
            case CMD_LAUNCH_GRIP:
                if (launch.type == CART_LAUNCH) {
                    printf("Killing cartesian mode launch...\n");
                    err_code = kill_launch(CART_LAUNCH, DEFAULT_KILL_TIMEOUT);
                }
                if (launch.type == POL_LAUNCH) {
                    printf("Killing polar mode launch...\n");
                    err_code = kill_launch(POL_LAUNCH, DEFAULT_KILL_TIMEOUT);
                }
                launch.type = GRIP_LAUNCH;
                do {
                    printf("Launching robot in gripper mode...\n");
                    err_code = launch_seq(GRIP_LAUNCH, 0, NULL, 1);
                } while(err_code != OK);
                break;
            case CMD_OPEN_GRIP:
                if (launch.type == GRIP_LAUNCH) {
                    do {
                        printf("Openning gripper...\n");
                        err_code = launch_seq(OPENG_LAUNCH, user_cmd.progmode, user_cmd.imgname, 1.0);
                    } while(err_code != OK);
                }
                else {
                    printf("Launch GRIP mode before trying to open the gripper.\n");
                }
                break;
            case CMD_CLOSE_GRIP:
                if (launch.type == GRIP_LAUNCH) {
                    do {
                        printf("Closing gripper...\n");
                        err_code = launch_seq(CLOSEG_LAUNCH, user_cmd.progmode, user_cmd.imgname, 1.0);
                    } while(err_code != OK);
                }
                else {
                    printf("Launch GRIP mode before trying to close the gripper.\n");
                }
                break;
            case CMD_LAUNCH_TEST:
                if (test_running == TRUE) {
                    printf("Killing test launch sequence...\n");
                    err_code = kill_launch(TEST_LAUNCH, DEFAULT_KILL_TIMEOUT);
                    test_running = FALSE;
                }
                else {
                    test_running = TRUE;
                    do {
                        printf("Running test launch sequence...\n");
                        err_code = launch_seq(TEST_LAUNCH, 0, NULL, 1);
                    } while(err_code != OK);
                }
                break;
            case CMD_RUN:
                program_running = TRUE;
                do {
                    printf("Running program...\n");
                    err_code = launch_seq(PROG_LAUNCH, user_cmd.progmode, user_cmd.imgname, 1);
                } while(err_code != OK);
                break;
            case CMD_RUN_STOP:
                if (program_running == TRUE) {
                    printf("Killing program...\n");
                    err_code = kill_launch(PROG_LAUNCH, DEFAULT_KILL_TIMEOUT);
                    program_running = FALSE;
                }
                break;
            case CMD_EMERGANCY_STOP:
                printf("EMERGANCY STOP!\nKilling: program, gripper open, gripper close, gripper launch, and test sequence.\n");
                err_code = kill_launch(PROG_LAUNCH, 0);
                err_code = kill_launch(OPENG_LAUNCH, 0);
                err_code = kill_launch(CLOSEG_LAUNCH, 0);
                err_code = kill_launch(GRIP_LAUNCH, 0);
                err_code = kill_launch(TEST_LAUNCH, 0);
                program_running = FALSE;
                test_running = FALSE;
                if (launch.type == GRIP_LAUNCH)
                    launch.type = NO_LAUNCH;
                break;
            default:
                printf("Unknown command - '%s' for help.\n", HELP_CMD_STR);
                break;
        }
    }
    return err_code;
}

/* Cleanup */
static void cleanup() {
    printf("Exiting...\n");
    cleanup_launches();
    exit(OK);
}

