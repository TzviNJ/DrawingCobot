# DrawbotRunner
C low-level interface for running drawing cobot.

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

The Drawbot Runner will also be able to be used as a python library. The most important file for that is launch.c which includes the three callable functions:
1) launchseq(launchtype, progmode, img) - launches the sequence specified by launchtype with the paramaters progmode and img if relevant.
2) killlaunch(launchtype, timeout) - kills the sequence specified by launchtype using timeout before hardkilling if softkilling doesn't work.
4) cleanup_launches() - cleanup for clean exit (and kills all remaining sub processes.
