# Drawing Cobot
Tzvi Jungreis and Shahar Yohananov  
Senior Project - Tel Aviv University
## Introduction
Drawing Cobot uses Python over a ROS2 environment, controlling a general-purpose Universal UR3E Cobot arm, in order to draw an SVG image uploaded via a user interface.
## File Division
The project consists of two parts: (1) the ROS2 controller, and (2) the user interface.  
### proj_ws folder
This folder is the ROS2 workspace for Drawing Cobot. It contains the ROS2 controller node and the image parser.  
A detailed description of the files in the workspace can be found under proj_ws/src/drawing_cobot/drawing_cobot/README.md
### user_interface folder
This folder contains the UIs for running the program. It contains both the Low Level C UI and the High Level Python UI.
A detailed description of the files in this folder and of the Low Level C UI can be found under user_interface/README.md
## Setup and Installation
After downloading this Git Repository,
- Controller: Make sure that ROS2 Jazzy and the Universal ROS2 API is installed. Otherwise, no setup necessary.
- Low Level C UI: Run 'Make' in user_interface (use of this UI is not suggested without perusing user_interface/README.md).
- High Level Python GUI: No extra setup necessary (see detailed instructions about updating library after modifications).
## Execution and Usage
In order to run the program, navigate to the user_interface folder.
In the Bash command line, execute:

```console
python3 UI.py
```
This opens the High Level Python GUI (for the Low Level C UI see user_interface/README.md).
## System Requirements
- Ubuntu Linux computer [for running the code] (we used version 24.04)
- Python3
- ROS2 Jazzy distribution
- Universal UR3E Cobot arm
- Robotiq Gripper [attached to the arm for holding the marker]
