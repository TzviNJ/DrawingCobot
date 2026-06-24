##########################
# @file: main.py
# @breif: robctl main
# Date Created: 2.12.25
# Version: 1.3.0
##########################

import rclpy
import numpy as np
import pandas as pd
from rclpy.node import Node
from .submodules.debug import Debugger
from .submodules.imgparser import Image
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint
from geometry_msgs.msg import TwistStamped, WrenchStamped
from moveit_msgs.srv import ServoCommandType
from tf2_ros import Buffer, TransformListener
import time
from os.path import isfile
from os import remove

#########################
# Global Variables      #
#########################
SPIN_TIMEOUT = 0.2 # sec
TRAJ_BUF_SIZE = 100
FORCE_BUF_SIZE = 100
# Bounds:
Y_FORCE_TH_STOP = -7.0
Y_FORCE_TH_START = -2.5
#########################

class Robctl(Node):
    def __init__(self):
        super().__init__('Robctl')
        self.log_mode = False
        self.f_frequency = 10.2
        self.T_period = 1/self.f_frequency
        if self.log_mode:
            self.drawlog = pd.DataFrame(data={
                "timestamp": [],
                "target_x": [],
                "target_y": [],
                "current_x": [],
                "current_y": [],
                "velocity_x": [],
                "velocity_y": []
            })
            self.pressinglog = pd.DataFrame(data={
                "timestamp": [],
                "target_effort": [],
                "current_effort": [],
                "velocity_z": [],
                "current_z": []
            })
        self.curr_pos = np.zeros(3)
        self.trajectory_publisher_ = self.create_publisher(JointTrajectory, '/scaled_joint_trajectory_controller/joint_trajectory', TRAJ_BUF_SIZE)
        self.force_subscriber = self.create_subscription(WrenchStamped, '/force_torque_sensor_broadcaster/wrench', self.wrench_callback, FORCE_BUF_SIZE)
        self.joint_names = ['shoulder_pan_joint', 'shoulder_lift_joint', 'elbow_joint', 'wrist_1_joint', 'wrist_2_joint', 'wrist_3_joint']
        self.cart_publisher = self.create_publisher(TwistStamped, '/servo_node/delta_twist_cmds', TRAJ_BUF_SIZE)
        # The following four lines are adapted from the tutorials https://docs.ros.org/en/jazzy/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Py-Service-And-Client.html and chatgpt
        self.cli = self.create_client(ServoCommandType, '/servo_node/switch_command_type')
        self.req = ServoCommandType.Request()
        self.req.command_type = 1 # Change Twist mode to required Cartesian mode
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.under_pressing = False
        self.over_pressing = False
        self.pendown = False
        # Constants
        self.shutdown_position_ang = [4.83, -1.91, -1.78, 1.89, -1.58, 3.14]
        self.start_write = [4.700, -2.490, -0.967, 3.437, -1.574, 3.138]
        self.top_left = [5.212, -2.313, -1.266, 3.568, -1.060, 3.129]
        # Paramaters
        self.declare_parameter('progmode', 0)
        self.progmode = self.get_parameter('progmode').value
        self.declare_parameter('img', "No_Image_Provided")
        self.img = self.get_parameter('img').value
        self.declare_parameter('resolution', 1.0)
        try:
            self.image_resolution = float(self.get_parameter('resolution').value)
        except:
            self.image_resolution = 0.0
        self.crashfile_path = "crash.log"
        # Modes
        self.debug_mode = False
        self.startup = False
        self.run = False
        self.shutdown = False
        self.print_position = False

    def DrawCurve(self, cmd, rclpy):
        ''' Execute a Command '''
        if self.debug_mode:
            print(cmd)
        while (not self.tcp_lookup()):
            rclpy.spin_once(self, timeout_sec=0.1)
        match cmd.gettype():
            case "M":
                destination = cmd.getcoords()
                if self.pendown:
                    self.move_to(None, None, "up")
                    self.pendown = False
                    rclpy.spin_once(self, timeout_sec=SPIN_TIMEOUT)
                if not self.move_to(destination[0], destination[1]):
                    print("Aborting.")
                    return False
                if not self.move_to(None, None, "down"):
                    print("Aborting.")
                    return False
                self.pendown = True
                rclpy.spin_once(self, timeout_sec=SPIN_TIMEOUT)
            case "L":
                self.pendown = True
                destination = cmd.getcoords()
                if not self.move_to(destination[0], destination[1]):
                    print("Aborting.")
            case "C":
                self.pendown = True
                destination = cmd.getcoords()
                if not self.move_cubic_bezier(np.array([self.curr_pos[0], self.curr_pos[1]]), 
                                          np.array([destination[0], destination[1]]),
                                          np.array([destination[2], destination[3]]),
                                          np.array([destination[4], destination[5]])):
                    print("Aborting.")
            case "Q":
                self.pendown = True
                destination = cmd.getcoords()
                if not self.move_quadratic_bezier(np.array([self.curr_pos[0], self.curr_pos[1]]), 
                                          np.array([destination[0], destination[1]]),
                                          np.array([destination[2], destination[3]])):
                    print("Aborting.")
            case "Ell":
                self.pendown = True
                destination = cmd.getcoords()
                if not self.move_ell(destination[0], destination[1], np.array([destination[2], destination[3]])):
                    print("Aborting.")
            case _:
                return False
        return True

    def move_to_resting_pose(self, time_to_goal, resting_pose_name="SHUTDOWN"):
        ''' Move to one of the pre-programmed resting poses '''
        if (resting_pose_name == "SHUTDOWN"):
            pose_coord = self.shutdown_position_ang
        elif (resting_pose_name == "WRITING"):
            pose_coord = self.start_write
        elif (resting_pose_name == "TOPLEFT"):
            pose_coord = self.top_left
        self.move_to_pose_ang(pose_coord, time_to_goal)
        self.get_logger().info(f'Moving to {resting_pose_name} position: {pose_coord}. Taking {time_to_goal} seconds.')

    def move_to_pose_ang(self, target_positions_ang, time_to_goal):
        ''' Publishes to joint_trajectory_controller in order to move to absolute angular pose. '''
        msg = JointTrajectory()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.joint_names = self.joint_names
        point = JointTrajectoryPoint()
        point.positions = target_positions_ang
        point.velocities = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        point.accelerations = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        point.time_from_start.sec = time_to_goal # Reach goal in time_to_goal seconds
        msg.points.append(point)
        self.trajectory_publisher_.publish(msg)
        
    def move_cubic_bezier(self, p_i, p_c1, p_c2, p_f):
        '''This function samples points on a cubic bezier, and moves in straight lines between samples.
        The sampling_res parameter determines how many points are sampled.'''
        sampling_res = 10
        for i in range(1,sampling_res+1):
            t = i/sampling_res
            B_t = (1-t)**3*p_i+3*(1-t)**2*t*p_c1+3*(1-t)*t**2*p_c2+t**3*p_f
            if not self.move_to(B_t[0], B_t[1]):
                return False
        return True   
    
    def move_quadratic_bezier(self, p_i, p_c1, p_f):
        '''This function samples points on a quadratic bezier, and moves in straight lines between samples.
        The sampling_res parameter determines how many points are sampled.'''
        sampling_res = 10
        for i in range(1,sampling_res+1):
            t = i/sampling_res
            B_t = (1-t)**2*p_i+2*(1-t)*t*p_c1+t**2*p_f
            if not self.move_to(B_t[0], B_t[1]):
                return False
        return True  

    def move_ell(self, rx, ry, c):
        '''This function samples an points on ellipse, and moves in straight lines between samples.
        The sampling_res parameter determines how many points are sampled.'''
        sampling_res = 40
        for i in range(1,sampling_res+1):
            t = i/sampling_res
            B_t = c + np.array([rx*np.cos(2*np.pi*t), ry*np.sin(2*np.pi*t)])
            if not self.move_to(B_t[0], B_t[1]):
                return False
        return True

    def log_velocities(self, timestamp, goal, pos, v):
        '''This function stores in the log the current velocities'''
        new_data = pd.DataFrame(data={
            "timestamp": [timestamp],
            "target_x": [goal[0]],
            "target_y": [goal[1]],
            "current_x": [pos[0]],
            "current_y": [pos[1]],
            "velocity_x": [v[0]],
            "velocity_y": [v[1]]
        })
        self.drawlog = pd.concat([self.drawlog, new_data])

    def log_efforts(self, timestamp, goal, effort, vz, z):
        '''This function stores in the log the current efforts'''
        new_data = pd.DataFrame(data={
            "timestamp": [timestamp],
            "target_effort": [goal],
            "current_effort": [effort],
            "velocity_z": [vz],
            "current_z": [z]
        })
        self.pressinglog = pd.concat([self.pressinglog, new_data])

    def update_logfiles(self):
        '''This function stores the log's content in a csv file.'''
        self.pressinglog.to_csv("pressing_log.csv")
        self.drawlog.to_csv("draw_log.csv")        

    def move_to(self, x, y, z=None):
        ''' This function builds a message to move the pen in the desired direction.'''
        # v/f<d => v<10d
        normal_factor = 0.07
        dec_factor = 0.007
        critical_dist = 0.0005
        dec_dist = 0.01
        msg = TwistStamped()
        msg.header.frame_id = 'base_link'
        if (x == None and y == None):
            msg.twist.linear.x = 0.0
            msg.twist.linear.y = 0.0
            self.under_pressing = True
            if (z == "up"):
                z_vel = 0.03
                for i in range(5):
                    msg.header.stamp = self.get_clock().now().to_msg()
                    msg.twist.linear.z = z_vel
                    self.cart_publisher.publish(msg)
                    start_time = time.time()
                    while (time.time()-start_time) < self.T_period:
                        pass
                msg.header.stamp = self.get_clock().now().to_msg()
                msg.twist.linear.z = 0.0
                self.cart_publisher.publish(msg)
            else:
                z_vel = -0.03
                start_time = time.time()
                while (self.under_pressing):
                    msg.header.stamp = self.get_clock().now().to_msg()
                    msg.twist.linear.z = z_vel
                    self.cart_publisher.publish(msg)
                    if self.log_mode:
                        self.log_efforts(time.time(), Y_FORCE_TH_START, self.force.y, msg.twist.linear.z, self.curr_pos[2])
                    while (time.time()-start_time) < self.T_period:
                        rclpy.spin_once(self, timeout_sec=0.01) # VERY IMPORTANT SO THAT CALLBACK WORKS
                    start_time = time.time()
                if self.debug_mode:
                    self.get_logger().info("Contact with page.")
                msg.header.stamp = self.get_clock().now().to_msg()
                msg.twist.linear.z = 0.0
                self.cart_publisher.publish(msg)
                if self.log_mode:
                    self.log_efforts(time.time(), Y_FORCE_TH_START, self.force.y, msg.twist.linear.z, self.curr_pos[2])
            rclpy.spin_once(self, timeout_sec=0.1)
            return True
        while (not self.tcp_lookup()):
            rclpy.spin_once(self, timeout_sec=0.1)
        if self.debug_mode:
            print(f"Beginning move from position: x={self.curr_pos[0]}, y={self.curr_pos[1]}, z={self.curr_pos[2]}")
            rclpy.spin_once(self, timeout_sec=0.1)
        if z == None:
            z = self.curr_pos[2]
        goal = np.array([x, y, z])
        diff = (goal - self.curr_pos)[0:2]
        start_time = time.time()
        while np.linalg.norm(diff) > critical_dist:
            diff_dir = diff/np.linalg.norm(diff) #normalize diff
            msg.header.frame_id = 'base_link'
            if np.linalg.norm(diff) < critical_dist: # stop
                msg.twist.linear.x = 0.0
                msg.twist.linear.y = 0.0
                if self.debug_mode:
                    print("Critical distance!")
                    print(f"Position: x={self.curr_pos[0]}, y={self.curr_pos[1]}, z={self.curr_pos[2]}")
                msg.header.stamp = self.get_clock().now().to_msg()
                self.cart_publisher.publish(msg)
                if self.log_mode:
                    self.log_velocities(time.time(), goal, self.curr_pos, np.array([msg.twist.linear.x, msg.twist.linear.y]))
                if self.debug_mode:
                    self.get_logger().info(f"Moved successfully to position: x={x:.5f}, y={y:.5f}, z={z:.5f}")
                return True
            elif np.linalg.norm(diff) < dec_dist: # slow down
                msg.twist.linear.x = dec_factor*diff_dir[0]
                msg.twist.linear.y = dec_factor*diff_dir[1]
            else: # move in constant speed
                msg.twist.linear.x = normal_factor*diff_dir[0]
                msg.twist.linear.y = normal_factor*diff_dir[1]
            msg.twist.linear.z = (self.pendown) * ((self.over_pressing) * (0.01) + (self.under_pressing and not self.over_pressing) * (-0.01))
            msg.header.stamp = self.get_clock().now().to_msg()
            self.cart_publisher.publish(msg)
            if self.log_mode:
                self.log_velocities(time.time(), goal, self.curr_pos, np.array([msg.twist.linear.x, msg.twist.linear.y]))
                self.log_efforts(time.time(), 0.5*((Y_FORCE_TH_START)+(Y_FORCE_TH_STOP)), self.force.y, msg.twist.linear.z, self.curr_pos[2])
            # 10 Hz:
            while (time.time()-start_time) < self.T_period:
                rclpy.spin_once(self, timeout_sec=0.01) # VERY IMPORTANT SO THAT CALLBACK WORKS
            start_time = time.time()
            if (not self.tcp_lookup()):
                print("Error in TCP lookup.")
            diff = (goal - self.curr_pos)[0:2]
        msg.twist.linear.x = 0.0
        msg.twist.linear.y = 0.0
        msg.header.stamp = self.get_clock().now().to_msg()        
        self.cart_publisher.publish(msg)
        if self.log_mode:
            self.log_velocities(time.time(), goal, self.curr_pos, np.array([msg.twist.linear.x, msg.twist.linear.y]))
        if self.debug_mode:
            self.get_logger().info(f"Moved successfully to position: x={x:.5f}, y={y:.5f}, z={z:.5f}")
        return True
    
    def wrench_callback(self, msg: WrenchStamped):
        '''This function is responsible for measuring forces applied on pen.'''
        self.force = msg.wrench.force
        self.torque = msg.wrench.torque
        self.over_pressing = (self.force.y < Y_FORCE_TH_STOP)
        self.under_pressing = (self.force.y > Y_FORCE_TH_START)

    def tcp_lookup(self):
        ''' Return the current TCP (x,y,z) of tool. '''
        try:
            t = self.tf_buffer.lookup_transform(
                'base_link',
                'tool0',
                rclpy.time.Time()
            )
            p = t.transform.translation
            q = t.transform.rotation
            if (self.print_position):
                self.get_logger().info(f"Position: x={p.x:.5f}, y={p.y:.5f}, z={p.z:.5f}")
            self.curr_pos = np.array([p.x, p.y, p.z])
            return True
        except Exception as e:
            self.get_logger().warn(str(e))
            return False

def main(args=None):
    time_to_goal = 10 # seconds
    rclpy.init(args=args)
    robctl = Robctl()
    # Check running mode
    match robctl.progmode:
        case 0:
            # Run (no debug)
            print("Running without debug.")
            robctl.run = True
        case 1:
            # Run (with debug)
            print("Running with debug.")
            robctl.debug_mode = True
            robctl.run = True
        case 2:
            # Startup
            print("Startup (only).")
            robctl.startup = True
        case 3:
            # Shutdown
            print("Shutdown (only).")
            robctl.shutdown = True
        case 4:
            # Startup, run, shutdown
            print("Startup, Run, Shutdown..")
            robctl.startup = True
            robctl.run = True
            robctl.shutdown = True
        case 5:
            # Print x, y, z
            print("Get Position Mode.")
            robctl.print_position = True
            while(True):
                robctl.tcp_lookup()
                rclpy.spin_once(robctl, timeout_sec=1)
    debugger = Debugger(robctl.debug_mode)

    # Delay
    time.sleep(5)
    #rclpy.spin_once(robctl, timeout_sec=5)
    # Startup if needed
    if (robctl.startup):
        robctl.move_to_resting_pose(time_to_goal, "TOPLEFT")
        rclpy.spin_once(robctl, timeout_sec=time_to_goal)

    # Run robot if needed
    if (robctl.run and (robctl.img == "None")):
        print("Please specify an image to draw.")
    elif (robctl.image_resolution <= 0):
        print("Image resolution must be a positive number.")
    elif (robctl.run):
        # Change Servo mode to correct mode (based on chatgpt):
        while not robctl.cli.wait_for_service(timeout_sec=1.0):
            robctl.get_logger().info('ChangeToTwist not available, waiting again...')
        future = robctl.cli.call_async(robctl.req)
        rclpy.spin_until_future_complete(robctl, future)
        if future.result() is None:
            # No response
            raise RuntimeError(f"Service call failed: {future.exception()}")
        # Check if image exists
        try:
            with open(robctl.img, 'r') as f:
                print(f"Drawing Image: '{robctl.img}' {robctl.image_resolution:.2f}pp/mm x {robctl.image_resolution:.2f}pp/mm.")
                # Call image parser
                image = Image(f.read(), robctl.image_resolution, robctl.image_resolution)
                commands = image.getcommands()
                # Check if there is a previous crash file
                starting_idx = 0
                starting_x = None
                starting_y = None
                robctl.crashfile_path = robctl.img.split('.')[0] + ".crash"
                if isfile(robctl.crashfile_path):
                    with open(robctl.crashfile_path, 'r') as crashfile:
                        try:
                            crash_data = crashfile.readline().split(",")
                            starting_idx = int(crash_data[0])
                            starting_x = float(crash_data[1])
                            starting_y = float(crash_data[2])
                        except Exception as e:
                            print(e)
                            starting_idx = 0
                # Moving up the pen
                print("Moving pen up.")
                robctl.move_to(None, None, "up")
                robctl.pendown = False
                if starting_x != None and starting_y != None:
                    robctl.move_to(starting_x, starting_y)
                rclpy.spin_once(robctl, timeout_sec=SPIN_TIMEOUT)
                print("Beginning to draw picture.")
                picture_start_time = time.time()
                # Loop for Draw Curve
                for i, cmd in enumerate(commands[starting_idx:]):
                    robctl.DrawCurve(cmd, rclpy)
                    with open(robctl.crashfile_path, 'w') as crashfile:
                        crashfile.write(f"{starting_idx + i}, {robctl.curr_pos[0]:.4f},{robctl.curr_pos[1]:.4f}")
                    if robctl.log_mode:
                        robctl.update_logfiles()
                picture_finish_time = time.time()
                print(f"Total time taken to draw image: {picture_finish_time - picture_start_time}")
                # Remove crash file
                remove(robctl.crashfile_path)
                # Moving up the pen
                robctl.move_to(None, None, "up")
                robctl.pendown = False
                rclpy.spin_once(robctl, timeout_sec=SPIN_TIMEOUT)
        except FileNotFoundError:
                print(f"ERROR: Image '{robctl.img}' not found.")
    
    # Shutdown if needed
    if (robctl.shutdown):
        robctl.move_to_resting_pose(time_to_goal, "SHUTDOWN")

    try:
        rclpy.spin(robctl)
    except KeyboardInterrupt:
        pass
    finally:
        robctl.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()