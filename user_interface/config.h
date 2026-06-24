/*###############################
# @file: config.h
# @breif: configuration defines
# Date Created: 11.1.26
# Version 1.2.0
###############################*/

#ifndef CONFIG_H
#define CONFIG_H

#include "err_codes.h"
#include <stdio.h>
#include <stdlib.h>

/* Config params */
#define DEBUG_MODE              1       // 1 - Debug Mode, 0 - Not Debug Mode
#define PRINT_TO_STDERR         1       // 1 - Print to stderr on error, 0 - Don't
#define PRINT_INSTRUCTION       1       // 1 - Print instruction before execution (in Debug Mode)
#define TRUE                    1
#define FALSE                   0
#define MAX_CMD_LEN             256     // Max length of user command
#define CHUNK_SIZE              1
#define ALLOC_SIZE              20
#define MAX_PROGMODE_SIZE       30
#define MAX_IMGRES_SIZE         30
#define DECIMAL                 10
/* User commands */
#define EXIT_CMD_STR            "exit"  // User cmd text for exiting program
#define HELP_CMD_STR            "help"  // User cmd text for help
#define LAUNCH_CART_STR         "cart"  // User cmd text for launch cart
#define LAUNCH_POL_STR          "pol"   // User cmd text for launch pol
#define LAUNCH_GRIP_STR         "grip"  // User cmd text for launch grip
#define OPEN_GRIP_STR           "open"  // User cmd text for open gripper
#define CLOSE_GRIP_STR          "close" // User cmd text for close gripper
#define LAUNCH_PROG_STR         "run"   // User cmd text for launch program
#define STOP_PROG_STR           "stop"  // User cmd text for launch program
#define EMGNCY_STOP_STR         "e"     // User cmd text for EMERGANCY STOP
#define LAUNCH_TEST_STR         "test"  // User cmd text for launch test sequence
/* Launch instructions */
#define DEFAULT_KILL_TIMEOUT    0 // Seconds
#define ROBOT_IP                "robot_ip:=192.168.0.10"
#define UR_TYPE                 "ur_type:=ur3e"
#define LAUNCH_RVIZ             "launch_rviz:=false"
#define WS_DIR                  "/home/tzvij/Documents/Repositories/DrawingCobot/proj_ws/"
#define SETUP_PATH              "./install/setup.bash"
#define BASHRC_PATH             "~/.bashrc"
// Special instructions:
#define CHDIR                   "SPECIAL-CHDIR"
#define CHDIR_IDX               1
#define END_CHDIR               "SPECIAL-END_CHDIR"
#define WAITFOREXEC             "SPECIAL-WAIT4EXEC"
#define BGDISPLYALL             "SPECIAL-BGDISPLYALL"
#define DEL5                    "SPECIAL-DELAY5"
#define DELAY_AFTER_PIPE_LAUNCH 0 // Seconds
/* Defines */
#define READ_END                0
#define WRITE_END               1

/* Fatal error handler */
#if (PRINT_TO_STDERR == 1)
#define ERR_EXIT(STR, ERR_CODE) \
    do { \
        fprintf(stderr, "FATAL ERROR: "); \
        print_errcode(ERR_CODE, stderr); \
        fprintf(stderr, " - %s.\n", STR); \
        exit(ERR_CODE); \
    } while(0)
#else
#define ERR_EXIT(TYPE, ERR_CODE) \
    do { exit(ERR_CODE) } while(0)
#endif

#endif /*CONFIG_H*/
