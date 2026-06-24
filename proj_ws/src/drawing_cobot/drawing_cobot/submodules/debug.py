##########################
# @file: debug.py
# @breif: debug class
# Date Created: 2.12.25
# Version: 0.0.0
##########################

class Debugger():
    def __init__(self, is_on):
        self.debug_on = is_on
        if (self.debug_on):
            print("DEBUG_MODE: Debugger is on.")
        