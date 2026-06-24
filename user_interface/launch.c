/*############################################
# @file: launch.c
# @breif: functions for handling the processes
# Date Created: 16.3.26
# Version 1.2.0
##############################################
# Functions:
#   + launch_seq
#   + kill_launch
#   + cleanup_launches
# Dependancies:
#   + config.h - configuration and defines
#   + launch.h - launch struct definition
#                and function declarations
#   + err_codes.h - error code definitions
############################################*/

#include "config.h"
#include "launch.h"
#include "err_codes.h"
#include <string.h> // strcmp
#include <unistd.h> // Pipes/forks/execvps, etc.
#include <signal.h> // For kill signal
#include <sys/types.h> // pid_t
#include <sys/wait.h> // waitpid

/* Global Variables */
pid_t* pol_launch_pids = NULL;
int pol_num_pids = 0;
pid_t* cart_launch_pids = NULL;
int cart_num_pids = 0;
pid_t* grip_launch_pids = NULL;
int grip_num_pids = 0;
pid_t* grip_open_pids = NULL;
int open_num_pids = 0;
pid_t* grip_close_pids = NULL;
int close_num_pids = 0;
pid_t* prog_launch_pids = NULL;
int prog_num_pids = 0;
pid_t* test_launch_pids = NULL;
int test_num_pids = 0;

/*****************************************************
 * @name launch_seq
 * @brief Launch a specified robot mode.
 * @remarks Creates child processes for each process
 *      needed to launch the specified launchtype.
 *      In the case of NO_LAUNCH calls kill_launch
 *      on all launch types.
 * @param int launchtype (enum defined in launch.h)
 * @retval err_code (enum defined in err_codes.h)
*****************************************************/
int launch_seq(int launchtype, int progmode, char* img, double img_res) {
    Err_Codes err_code = OK;
    char progmode_str[MAX_PROGMODE_SIZE+1];
    char imgres_str[MAX_IMGRES_SIZE+1];
    char chunk[CHUNK_SIZE];
    char* buf = NULL;
    char* imgparamstr = NULL;
    size_t buf_size = 0;
    size_t total_read = 0;
    size_t bytes_read = 0;
    int chdir_flg = FALSE;
    int use_pipe = TRUE;
    const char* newdir = NULL;
    int i = 0;
    pid_t pid = -1;
    pid_t cat_pid = -1;
    int exec_retval = -1;
    int fd[2] = {0, 0}; // Pipe
    int status = 0;
    int* num_pids_ptr = NULL;
    pid_t** launch_pids_ptr = NULL;
    const char *(*launch_instr)[MAX_LAUNCH_ARGS] = NULL;
    const char *(*curr_instr)[MAX_LAUNCH_ARGS] = NULL;
    const char *(*init_strs)[] = NULL;
    const char **curr_init_str = NULL;
    const char *cart_instr[][MAX_LAUNCH_ARGS] = CART_LAUNCH_SEQ;
    const char *cart_str[] = CART_INIT_STR;
    const char *pol_instr[][MAX_LAUNCH_ARGS] = POL_LAUNCH_SEQ;
    const char *pol_str[] = POL_INIT_STR;
    const char *grip_instr[][MAX_LAUNCH_ARGS] = GRIP_LAUNCH_SEQ;
    const char *grip_str[] = GRIP_INIT_STR;
    const char *open_instr[][MAX_LAUNCH_ARGS] = GRIP_OPEN_SEQ;
    const char *open_str[] = OPEN_INIT_STR;
    const char *close_instr[][MAX_LAUNCH_ARGS] = GRIP_CLOSE_SEQ;
    const char *close_str[] = CLOSE_INIT_STR;
    const char *prog_instr[][MAX_LAUNCH_ARGS] = RUN_SEQ;
    const char *prog_str[] = RUN_INIT_STR;
    const char *test_instr[][MAX_LAUNCH_ARGS] = TEST_SEQ;
    const char *test_str[] = TEST_INIT_STR;
    switch(launchtype) {
        case NO_LAUNCH:
            /* Kill all launches and return. */
            for (i = NO_LAUNCH+1; i < UNKNOWN_LAUNCH; i++) {
                err_code = kill_launch(i, DEFAULT_KILL_TIMEOUT);
                if (err_code != OK)
                    return err_code;
            }
            return err_code;
            break;
        case POL_LAUNCH:
            launch_instr = pol_instr;
            num_pids_ptr = &pol_num_pids;
            launch_pids_ptr = &pol_launch_pids;
            init_strs = &pol_str;
            break;
        case CART_LAUNCH:
            launch_instr = cart_instr;
            num_pids_ptr = &cart_num_pids;
            launch_pids_ptr = &cart_launch_pids;
            init_strs = &cart_str;
            break;
        case GRIP_LAUNCH:
            launch_instr = grip_instr;
            num_pids_ptr = &grip_num_pids;
            launch_pids_ptr = &grip_launch_pids;
            init_strs = &grip_str;
            break;
        case OPENG_LAUNCH:
            launch_instr = open_instr;
            num_pids_ptr = &open_num_pids;
            launch_pids_ptr = &grip_open_pids;
            init_strs = &open_str;
            break;
        case CLOSEG_LAUNCH:
            launch_instr = close_instr;
            num_pids_ptr = &close_num_pids;
            launch_pids_ptr = &grip_close_pids;
            init_strs = &close_str;
            break;
        case TEST_LAUNCH:
            launch_instr = test_instr;
            num_pids_ptr = &test_num_pids;
            launch_pids_ptr = &test_launch_pids;
            init_strs = &test_str;
            break;
        case PROG_LAUNCH:
            launch_instr = prog_instr;
            num_pids_ptr = &prog_num_pids;
            launch_pids_ptr = &prog_launch_pids;
            init_strs = &prog_str;
            /* Add ROS args to instructions */
            sprintf(progmode_str, "progmode:=%d", progmode);
            launch_instr[NODE_INSTR_IDX][PROGMODE_IDX] = (const char*) progmode_str;
            if (img != NULL && strlen(img) > 0) {
                imgparamstr = (char*) malloc((strlen("img:=") + strlen(img) + 1)*sizeof(char));
                if (imgparamstr == NULL)
                    ERR_EXIT("out of memory", ERR_MEMORY);
                sprintf(imgparamstr, "img:=%s", img);
                launch_instr[NODE_INSTR_IDX][IMG_IDX] = (const char*) imgparamstr;
                sprintf(imgres_str, "resolution:=%.05lf", img_res);
                launch_instr[NODE_INSTR_IDX][RES_IDX] = (const char*) imgres_str;
            }
            break;
        case ERR_LAUNCH:     // ERR_LAUNCH treated as default
        case UNKNOWN_LAUNCH: // UNKNOWN_LAUNCH treated as default
        default:
            #if (PRINT_TO_STDERR == 1)
            fprintf(stderr, "launch_seq(): '%d' is an invalid launch type.\n", launchtype);
            #endif
            return ERR_BAD_ARGS;
            break;
    }
    /* At this point one of the launch types should occur and the instructions are stored in launch_instr. */
    /* Loop through each of the launch instructions and execute: */
    if (num_pids_ptr == NULL || launch_pids_ptr == NULL)
        return ERR_BAD_ARGS;
    for (curr_instr = launch_instr, curr_init_str = *init_strs; **curr_instr != NULL; curr_instr++, curr_init_str++) {
        /* Check what the instruction is */
        if (strcmp((*curr_instr)[0], CHDIR) == 0) {
            /* Change working directory */
            chdir_flg = TRUE;
            newdir = (*curr_instr)[CHDIR_IDX];
        }
        else if (strcmp((*curr_instr)[0], END_CHDIR) == 0) {
            chdir_flg = FALSE;
            newdir = NULL;
        }
        else {
            /* Not a special instruction - execute as normal */
            /* Check if a pipe is needed */
            if ((*curr_init_str == NULL) || (strcmp(*curr_init_str, WAITFOREXEC) == 0) || (strcmp(*curr_init_str, BGDISPLYALL) == 0) || (strcmp(*curr_init_str, DEL5) == 0))
                use_pipe = FALSE;
            else
                use_pipe = TRUE;
            if (use_pipe == TRUE) {
                /* Create a pipe */
                if (pipe(fd) < 0)
                    return ERR_PIPE;
            }
            #if (DEBUG_MODE == 1 && PRINT_INSTRUCTION == 1)
            /* Print instruction */
            printf("Executing instruction: ");
            for(char *const *arg = (char* const*)*curr_instr; *arg != NULL; arg++)
                printf("%s ", *arg);
            printf("- %s.\n", (use_pipe == TRUE) ? "With pipe" : "Without pipe");
            #endif
            /* Fork to execute instruction */
            pid = fork();            
            if (pid > 0) {
                /* Parent Process - pid is the child PID */
                if ((*curr_init_str != NULL) && (strcmp(*curr_init_str, WAITFOREXEC) == 0)) {
                    /* Wait for child to finish */
                    waitpid(pid, &status, 0);
                }
                else {
                    /* Add child pid to list of running processes for the given launchtype. */
                    (*num_pids_ptr)++;
                    (*launch_pids_ptr) = (pid_t*) realloc(*launch_pids_ptr, (*num_pids_ptr)*sizeof(pid_t));
                    if ((*launch_pids_ptr) == NULL)
                        ERR_EXIT("realloc failed", ERR_MEMORY);
                    (*launch_pids_ptr)[(*num_pids_ptr)-1] = pid;
                    if (use_pipe == TRUE) {
                        /* Use pipe to make sure that child is ready for next step */
                        close(fd[WRITE_END]); // Close write end of pipe
                        /* Can now use fd[0] to monitor the output of the child. */
                        if (*curr_init_str != NULL) {
                            // Wait until fd[0] outputs expected string
                            total_read = 0;
                            while((bytes_read = read(fd[READ_END], chunk, CHUNK_SIZE)) > 0) {
                                /* Read a chunk of data from pipe and append it to total read data */
                                while (buf_size < (total_read + bytes_read + 1)*sizeof(char)) {
                                    buf_size += ALLOC_SIZE*sizeof(char);
                                    buf = (char*) realloc(buf, buf_size);
                                    if (buf == NULL)
                                        ERR_EXIT("realloc failed", ERR_MEMORY);
                                }
                                memcpy(buf + total_read, chunk, bytes_read);
                                total_read += bytes_read;
                                buf[total_read] = '\0';
                                /* Check if the required string has been read yet */
                                if (strstr(buf, *curr_init_str) != NULL) {
                                    break;
                                }
                            }
                            if (bytes_read <= 0) {
                                /* Either an error in the pipe or pipe closed before receiving target */
                                #if (DEBUG_MODE == 1)
                                printf("DEBUG_MODE: Process finished initializing (WITHOUT FINISH STRING).\n");
                                #endif
                            }
                            if (buf != NULL) {
                                free(buf);
                                buf = NULL;
                                buf_size = 0;
                            }
                            /* At this point redirect the pipe to STDOUT by making a child process which copies */
                            cat_pid = fork();
                            if (cat_pid < 0) {
                                /* Fork Error */
                                #if (PRINT_TO_STDERR == 1)
                                fprintf(stderr, "Warning: Cannot monitor errors from child process.\n");
                                #endif
                            }
                            else if (cat_pid > 0) {
                                /* Parent - add the PID to the list */
                                (*num_pids_ptr)++;
                                (*launch_pids_ptr) = (pid_t*) realloc(*launch_pids_ptr, (*num_pids_ptr)*sizeof(pid_t));
                                if ((*launch_pids_ptr) == NULL)
                                    ERR_EXIT("realloc failed", ERR_MEMORY);
                                (*launch_pids_ptr)[(*num_pids_ptr)-1] = cat_pid;
                            }
                            else {
                                /* Child */
                                dup2(fd[READ_END], STDIN_FILENO);
                                execlp("cat", "cat", NULL);   
                            }
                            /* Delay predetermined number of seconds before going to next launch instruction in case not yet completely initialized. */
                            #if (DELAY_AFTER_PIPE_LAUNCH > 0)
                                sleep(DELAY_AFTER_PIPE_LAUNCH); // In seconds
                            #endif
                        }
                        close(fd[READ_END]); // The read end of the pipe can now be closed
                    } /* End if pipe */
                } /* End no wait for exec */
            } /* End Parent */
            else if (pid == 0) {
                /* Child Process - execute instruction */
                setpgid(0, 0);
                if (use_pipe == TRUE) {
                    close(fd[READ_END]); // Close read end of pipe
                    dup2(fd[WRITE_END], STDOUT_FILENO); // Redirect the child's STDOUT to the pipe
                    close(fd[WRITE_END]); // Close write end of pipe
                }
                if (chdir_flg == TRUE)
                    chdir(newdir); // Change the working dir for this instruction only
                exec_retval = execvp((*curr_instr)[0], (char* const*)*curr_instr);
                exit(exec_retval); // execvp failed
            }
            else {
                return ERR_FORK;
            }
            if ((*curr_init_str != NULL) && (strcmp(*curr_init_str, DEL5) == 0))
                sleep(10);
        }
    }
    if (imgparamstr != NULL)
        free(imgparamstr);
    return err_code;
}

/*****************************************************
 * @name kill_launch
 * @brief Kill all proc. in specified robot launch mode.
 * @remarks Given a launchtype, kills all processes
 *     created when mode was launched. Sends graceful
 *     kill signal for each process but after timeout
 *     hardkills the process if still running.
 * @param int launchtype (enum defined in launch.h)
 * @param int timeout (in seconds)
 * @retval err_code (enum defined in err_codes.h)
*****************************************************/
int kill_launch(int launchtype, int timeout) {
    Err_Codes err_code = OK;
    pid_t return_pid;
    int status = 0;
    int i = 0;
    int* num_pids_ptr = NULL;
    pid_t** launch_pids_ptr = NULL;
    switch(launchtype) {
        case NO_LAUNCH:
            // You can't kill no launch... 
            return ERR_BAD_ARGS;
            break;
        case POL_LAUNCH:
            num_pids_ptr = &pol_num_pids;
            launch_pids_ptr = &pol_launch_pids;
            break;
        case CART_LAUNCH:
            num_pids_ptr = &cart_num_pids;
            launch_pids_ptr = &cart_launch_pids;
            break;
        case GRIP_LAUNCH:
            num_pids_ptr = &grip_num_pids;
            launch_pids_ptr = &grip_launch_pids;
            break;
        case OPENG_LAUNCH:
            num_pids_ptr = &open_num_pids;
            launch_pids_ptr = &grip_open_pids;
            break;
        case CLOSEG_LAUNCH:
            num_pids_ptr = &close_num_pids;
            launch_pids_ptr = &grip_close_pids;
            break;
        case PROG_LAUNCH:
            num_pids_ptr = &prog_num_pids;
            launch_pids_ptr = &prog_launch_pids;
            break;
        case TEST_LAUNCH:
            num_pids_ptr = &test_num_pids;
            launch_pids_ptr = &test_launch_pids;
            break;
        case ERR_LAUNCH:     // ERR_LAUNCH treated as default
        case UNKNOWN_LAUNCH: // UNKNOWN_LAUNCH treated as default
        default:
            #if (PRINT_TO_STDERR == 1)
            fprintf(stderr, "launch_seq(): '%d' is an invalid launch type.\n", launchtype);
            #endif
            return ERR_BAD_ARGS;
            break;
    }
    if (num_pids_ptr == NULL || launch_pids_ptr == NULL)
        return ERR_BAD_ARGS;
    /* Kill processes from end to start of running list */
    for (i = (*num_pids_ptr)-1; i >= 0; i--) {
        /* Attempt to kill the process with CTRL^C */
        kill(-(*launch_pids_ptr)[i], SIGINT);
        /* Check if child is a zombie */
        return_pid = waitpid((*launch_pids_ptr)[i], &status, WNOHANG);
        if (return_pid == 0) {
            /* Child process is still running */
            if (timeout > 0)
                sleep(timeout); // Wait
            kill(-(*launch_pids_ptr)[i], SIGKILL); // Hardkill
            waitpid((*launch_pids_ptr)[i], &status, 0); // Reap
        }
        (*num_pids_ptr)--;
    }
    return err_code;
}

/*****************************************************
 * @name cleanup_launches
 * @brief Cleanup before exiting (free mem., etc.).
 * @remarks Free memory, kill all remaining procs.,
 *      close any files, etc.
*****************************************************/
void cleanup_launches() {
    int i = 0;
    /* Kill all launches still alive */
    for (i = NO_LAUNCH+1; i < UNKNOWN_LAUNCH; i++) {
        Launch lnch;
        lnch.type = i;
        printf("Killing: ");
        print_launch_type(&lnch, stdout);
        printf("\n");
        kill_launch(i, DEFAULT_KILL_TIMEOUT);
    }
    /* Clean up dynamic memory */
    if (pol_launch_pids != NULL)
        free(pol_launch_pids);
    if (cart_launch_pids != NULL)
        free(cart_launch_pids);
    if (grip_launch_pids != NULL)
        free(grip_launch_pids);
    if (test_launch_pids != NULL)
        free(test_launch_pids);
}