# Drawing Cobot User Interface

## Introduction
This file describes the C low level interface, its API for the Python high level interface and the steps for setting up the module to be used in Python.

## Low level interface
Running the executable starts a command line interface with the following possible commands:
1) 'help'
2) 'exit' - kill all processes and exit.
3) 'cart' - launch the series of processes required for cartesian mode (and kill 'pol' and 'test').
4) 'pol' - launch the series of processes required for polar mode (and kill 'cart' and 'test').
5) 'grip' - currently does nothing. In the future may launch gripper mode.
6) 'run PROGMODE IMG' - runs the Drawing Cobot node with the paramaters PROGMODE (int) and IMG (filepath-str).
7) 'stop' - kills the program.
8) 'test' - toggles a launch sequence used for testing.
9) 'e' - EMERGANCY STOP - kills program, 'grip', and 'test' (in that order).

## The interface as a Python module
The Drawbot Runner will also be able to be used as a python library. The most important file for that is launch.c which includes the three callable functions:
1) launchseq(launchtype, progmode, img) - launches the sequence specified by launchtype with the paramaters progmode and img if relevant.
2) killlaunch(launchtype, timeout) - kills the sequence specified by launchtype using timeout before hardkilling if softkilling doesn't work.
4) cleanup_launches() - cleanup for clean exit (and kills all remaining sub processes.

## Setting up the python module
Inside the interface directory, there are two file responsible for the setup of the module - launchmodule.c and setup.py.

### launchmodule.c
This file includes the C functions of the low level interface and does the necessary conversion for Python.
It defines identical functions to the C low level interface that receive Python objects and return Python objects.
These functions receive the necessary arguments from Python, call the original C low level interface functions, and finally return their results as Python objects.

Additionally, this file wraps the functions with a data structure called RunnerMethods that stores their pointers and any necessary Information about them. This structure is stored at the runner_module struct that defines the name of the module and its methods. 

Finally, this file initializes the module with the function PyInit_DrawBotRunner.

### setup.py
This file compiles launch.c and launchmodule.c to create the Python module.
It is run using the bash command:
```console
  python3 setup.py build_ext --inplace
```
