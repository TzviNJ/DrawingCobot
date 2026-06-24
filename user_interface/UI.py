from tkinter import Tk, Label, Button, filedialog, Radiobutton, IntVar, StringVar, messagebox, Entry
from enum import Enum
from time import sleep
from PIL import Image, ImageTk
from DrawBotRunner import *

class LaunchType(Enum):
    NO_LAUNCH = 0
    POL_LAUNCH = 1
    CART_LAUNCH = 2
    GRIP_LAUNCH = 3
    OPENG_LAUNCH = 4
    CLOSEG_LAUNCH = 5
    PROG_LAUNCH = 6
    TEST_LAUNCH = 7 
    UNKNOWN_LAUNCH = 8
    ERR_LAUNCH = 9

class ProgMode(Enum):
    RUN_NODEBUG = 0
    RUN_DEBUG = 1
    STARTUP_MODE = 2
    SHUTDOWN_MODE = 3

root = Tk()
imgfile = StringVar()
imgfile.set('/')
imgres = StringVar()
imgres.set("1.0")
prog_mode = IntVar()
prog_mode.set(-1)
launch_type = IntVar()
current_launches = []
KILL_TIMEOUT = 0

def shutdown():
    ''' Called when the user clicks on the close button. '''
    if messagebox.askyesno("Exit", "Do you want to exit?"):
        cleanup_launches()
        root.destroy()

def kill_all_current_launches():
    ''' Kills all current launches'''
    for launch in current_launches:
        kill_launch(launch, KILL_TIMEOUT)

#Button functionalities
def b_click_start():
    ''' Called when the user clicks on the start button. '''
    try:
        image_resolution = float(imgres.get())
    except:
        image_resolution = 1.0
        imgres.set(str(image_resolution))
        messagebox.showerror("Attention", "Resolution must be float!")
    if prog_mode.get() == -1:
        messagebox.showerror("Attention", "No mode was selected!")
    elif prog_mode.get() == ProgMode.RUN_NODEBUG.value and (imgfile.get() == '/' or imgfile.get() == ''):
        messagebox.showerror("Attention", "No image was selected!")
    else:
        match prog_mode.get():
            case ProgMode.STARTUP_MODE.value:
                messagebox.showinfo("Attention", "Moving to startup position. This may take a few seconds.")

            case ProgMode.SHUTDOWN_MODE.value:
                messagebox.showinfo("Attention", "Moving to shutdown position. This may take a few seconds.")
                
            case ProgMode.RUN_NODEBUG.value:
                messagebox.showinfo("Attention", "Drawing picture. This may take a few seconds.")
        kill_all_current_launches()
        current_launches.append(launch_type.get())
        launch_seq(launch_type.get(), prog_mode.get(), imgfile.get(), image_resolution)
        sleep(10)
        current_launches.append(LaunchType.PROG_LAUNCH.value)
        launch_seq(LaunchType.PROG_LAUNCH.value, prog_mode.get(), imgfile.get(), image_resolution)

    #button_stop.config(state="disabled")
    print(prog_mode.get())

def b_click_stop():
    ''' Called when the user clicks on the stop button. '''
    print("Killing All")
    kill_all_current_launches()


def b_click_selimg():
    ''' Called when the user clicks on the select image button. '''
    global imgfile
    imgfile.set(filedialog.askopenfilename(initialdir="./", title="Select Image", filetypes=(("SVG Image", "*.svg"), ("All files", "*.*"))))
    path_label = Label(root, text=imgfile.get())
    path_label.grid(row=0,column=0, columnspan=3) 

def b_click_openg():
    ''' Called when the user clicks on the open gripper button. '''
    kill_all_current_launches()
    current_launches.append(LaunchType.GRIP_LAUNCH.value)
    launch_seq(LaunchType.GRIP_LAUNCH.value, ProgMode.RUN_NODEBUG.value, imgfile.get(), 1.0)
    sleep(10)
    launch_seq(LaunchType.OPENG_LAUNCH.value, ProgMode.RUN_NODEBUG.value, imgfile.get(), 1.0)

def b_click_closeg():
    ''' Called when the user clicks on the close gripper button. '''
    kill_all_current_launches()
    current_launches.append(LaunchType.GRIP_LAUNCH.value)
    launch_seq(LaunchType.GRIP_LAUNCH.value, ProgMode.RUN_NODEBUG.value, imgfile.get(), 1.0)
    sleep(10)
    launch_seq(LaunchType.CLOSEG_LAUNCH.value, ProgMode.RUN_NODEBUG.value, imgfile.get(), 1.0)

#Button definitions
button_start = Button(root, text="Start", command=b_click_start)
button_stop = Button(root, text="Stop", command=b_click_stop)
button_selimg = Button(root, text="Select Image", command=b_click_selimg)
button_openg = Button(root, text="Open Gripper", command=b_click_openg)
button_closeg = Button(root, text="Close Gripper", command=b_click_closeg)

#Button placements
button_start.grid(row=1, column=0)
button_stop.grid(row=1, column=1)
button_selimg.grid(row=1, column=2)
button_openg.grid(row=1, column=3)
button_closeg.grid(row=1, column=4)

#Text box for resolution
res_instr = Label(root, text="Img pp/mm:")
res_instr.grid(row=1, column=5)
resolution_text = Entry(root, textvariable=imgres)
resolution_text.grid(row=1, column=6)

#Radiobutton functionalities
def r_pol_launch():
    launch_type.set(LaunchType.POL_LAUNCH.value)
    print("Changed to startup launch")

def r_cart_launch():
    launch_type.set(LaunchType.CART_LAUNCH.value)
    print("Changed to cartesian launch")


#Radiobutton definitions
radio_startup = Radiobutton(root, text="Move to startup position", variable=prog_mode, value=ProgMode.STARTUP_MODE.value, command=r_pol_launch)
radio_shutdown = Radiobutton(root, text="Move to shutdown position", variable=prog_mode, value=ProgMode.SHUTDOWN_MODE.value, command=r_pol_launch)
radio_cartesian = Radiobutton(root, text="Draw picture", variable=prog_mode, value=ProgMode.RUN_NODEBUG.value, command=r_cart_launch)

#Radiobutton placements
radio_startup.grid(row=2, column=0, columnspan=3)
radio_shutdown.grid(row=3, column=0, columnspan=3)
radio_cartesian.grid(row=4, column=0, columnspan=3)

original_img = Image.open("./DrawingCobotLogo.png")
tk_img = ImageTk.PhotoImage(original_img)
label = Label(root, image=tk_img)
label.grid(row=2, column=3, columnspan=4, rowspan=3)
root.protocol("WM_DELETE_WINDOW", shutdown)
root.mainloop()