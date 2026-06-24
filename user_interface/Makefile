#################################
# @file: Makefile
# @breif: Makefile
# Date Created: 11.1.26
# Version 0.1.0
#################################

# Use gcc to compile:
CC = gcc
# A list of all object files used for the compilation of the executable:
OBJECTS = main.o launch.o
# Executable name:
EXENAME = run

$(EXENAME) : $(OBJECTS)
	$(CC) -Wall -Werror -g -o $(EXENAME) $(OBJECTS)

$(OBJECTS) : err_codes.h config.h # All object files depend on these files
main.o : main.c launch.h cmd_parser.h
	$(CC) -c main.c
launch.o : launch.c launch.h
	$(CC) -c launch.c

# Running 'make clean' should clean up all executables and object files
clean : 
	rm $(EXENAME) $(OBJECTS)
