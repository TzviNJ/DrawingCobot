/*###############################
# @file: launch.h
# @breif: launch struct
# Date Created: 11.1.26
# Version 1.2.0
###############################*/

#ifndef LAUNCH_H
#define LAUNCH_H

#include <stdio.h>

/* Function Declarations */
int launch_seq(int launchtype, int progmode, char* img, double img_res);
int kill_launch(int launchtype, int timeout);
void cleanup_launches();

/*****************************************************
 * @enum launch_type
 * @brief Launch sequences that can be performed.
*****************************************************/
typedef enum launch_type {
    NO_LAUNCH = 0,
    POL_LAUNCH,
    CART_LAUNCH,
    GRIP_LAUNCH,
    OPENG_LAUNCH,
    CLOSEG_LAUNCH,
    PROG_LAUNCH,
    TEST_LAUNCH,    // For testing
    UNKNOWN_LAUNCH,
    ERR_LAUNCH
} Launch_Type;

/*******************************************************
 * @struct launch
 * @brief Mode in which the robot is currently launched.
*******************************************************/
typedef struct launch {
    Launch_Type type;
} Launch;

/* Launch Sequence Defines */
#define MAX_LAUNCH_ARGS 12 // Number of arguments each launch instruction should have.
//ros2 launch ur_robot_driver ur_control.launch.py ur_type:=ur3e robot_ip:=192.168.0.10 initial_joint_controller:=scaled_joint_trajectory_controller launch_rviz:=false
//ros2 launch ur_moveit_config ur_moveit.launch.py ur_type:=ur3e launch_servo:=true launch_rviz:=false
#define CART_LAUNCH_SEQ { \
                         {"ros2", "launch", "ur_robot_driver", "ur_control.launch.py", UR_TYPE, ROBOT_IP, "initial_joint_controller:=scaled_joint_trajectory_controller", "headless_mode:=true", LAUNCH_RVIZ, NULL}, \
                         /* left out calibration , "kinematics_params_file:=\"${WS_DIR}/${CALIB_FNAME}.yaml\"" */ \
                         {"ros2", "launch", "ur_moveit_config", "ur_moveit.launch.py", UR_TYPE, "launch_servo:=true", LAUNCH_RVIZ, NULL}, \
                         /*{"ros2", "service", "call", "/io_and_status_controller/resend_robot_program", "std_srvs/srv/Trigger {}", NULL, NULL, NULL, NULL, NULL},*/ \
                         /*{"ros2", "service", "call", "/servo_node/switch_command_type moveit_msgs/srv/ServoCommandType", "\"{command_type: 1}\"", NULL},*/ \
                         NULL \
                        }
#define CART_INIT_STR   { \
                         NULL,/*"Robot connected to reverse interface. Ready to receive control commands.",*/\
                         NULL/*"process has finished cleanly"*/ \
                        }
#define POL_LAUNCH_SEQ  { \
                         {"ros2", "launch", "ur_robot_driver", "ur_control.launch.py", UR_TYPE, ROBOT_IP, "initial_joint_controller:=scaled_joint_trajectory_controller", LAUNCH_RVIZ, "headless_mode:=true", NULL}, \
                         /* left out calibration: , "kinematics_params_file:=\"${WS_DIR}/${CALIB_FNAME}.yaml\"" */ \
                         /*{"ros2", "service", "call", "/io_and_status_controller/resend_robot_program", "std_srvs/srv/Trigger {}", NULL, NULL, NULL, NULL, NULL},*/ \
                         NULL \
                        }
#define POL_INIT_STR    { \
                         NULL, /*"Robot connected to reverse interface. Ready to receive control commands.",*/ \
                         NULL \
                        }
#define GRIP_LAUNCH_SEQ { \
                         {"ros2", "launch", "ur_robot_driver", "ur_control.launch.py", UR_TYPE, ROBOT_IP, LAUNCH_RVIZ, "headless_mode:=true", NULL}, \
                         NULL \
                        }
#define GRIP_INIT_STR   { \
                         NULL /*"Robot connected to reverse interface. Ready to receive control commands.",*/ \
                        }
#define GRIP_OPEN_SEQ   { \
                         {"ros2", "service", "call", "/dashboard_client/load_program", "ur_dashboard_msgs/srv/Load", "{filename: \"gripper_open.urp\"}", NULL}, \
                         {"ros2", "service", "call" ,"/dashboard_client/play", "std_srvs/srv/Trigger", "{}", NULL}, \
                         NULL \
                        }
#define OPEN_INIT_STR   { \
                         WAITFOREXEC, \
                         WAITFOREXEC, \
                         NULL \
                        }
#define GRIP_CLOSE_SEQ   { \
                         {"ros2", "service", "call", "/dashboard_client/load_program", "ur_dashboard_msgs/srv/Load", "{filename: \"gripper_close.urp\"}", NULL}, \
                         {"ros2", "service", "call" ,"/dashboard_client/play", "std_srvs/srv/Trigger", "{}", NULL}, \
                         NULL \
                        }
#define CLOSE_INIT_STR   { \
                         WAITFOREXEC, \
                         WAITFOREXEC, \
                         NULL \
                        }
#define NODE_INSTR_IDX  3
#define PROGMODE_IDX    6
#define IMG_IDX         8
#define RES_IDX         10
#define RUN_SEQ         { \
                         {CHDIR, WS_DIR, NULL}, \
                         {"colcon", "build", "--symlink-install", NULL}, \
                         /*{"source", SETUP_PATH, NULL},*/ \
                         /*{"source", BASHRC_PATH, NULL},*/ \
                         {END_CHDIR, NULL}, \
                         {"ros2", "run", "drawing_cobot", "robctl", "--ros-args", "-p", "progmode:=None", "-p", "img:=None", "-p", "resolution:=None", NULL}, \
                         NULL \
                        }
#define RUN_INIT_STR    { \
                         NULL, \
                         WAITFOREXEC, \
                         NULL, \
                         BGDISPLYALL, \
                         NULL \
                        }
#define TEST_SEQ        { \
                         {"ls", "-l", NULL}, \
                         {"./testprog1", NULL}, \
                         {"./testprog2", NULL}, \
                         NULL \
                        }
#define TEST_INIT_STR   { \
                         WAITFOREXEC, \
                         "Program 1 initialized.", \
                         "Program 2 initialized." \
                        }

/*****************************************************************
 * @name print_launch_type
 * @brief Prints the name of the enum entry for given launch type.
 * @param launch Enum entry int to print as str.
 * @param fd File descriptor of target stream.
*****************************************************************/
static void print_launch_type(Launch *launch, FILE *fd) {
    switch(launch->type) {
        case NO_LAUNCH:
            fprintf(fd, "NO_LAUNCH");
            break;
        case POL_LAUNCH:
            fprintf(fd, "POL_LAUNCH");
            break;
        case CART_LAUNCH:
            fprintf(fd, "CART_LAUNCH");
            break;
        case GRIP_LAUNCH:
            fprintf(fd, "GRIP_LAUNCH");
            break;
        case OPENG_LAUNCH:
            fprintf(fd, "OPENG_LAUNCH");
            break;
        case CLOSEG_LAUNCH:
            fprintf(fd, "CLOSEG_LAUNCH");
            break;
        case PROG_LAUNCH:
            fprintf(fd, "PROG_LAUNCH");
            break;
        case ERR_LAUNCH:
            fprintf(fd, "ERR_LAUNCH");
            break;
        case TEST_LAUNCH:
            fprintf(fd, "TEST_LAUNCH");
            break;
        default:
            fprintf(fd, "UNKNOWN_LAUNCH");
            break;
    }
}

#endif /*LAUNCH_H*/