###########################
# @file: imgparser.py
# @breif: image parser
# version: 1.2.0
# Date of creation: 10.3.26
###########################

import xml.etree.ElementTree as ET
import sys

class Image:
    def __init__(self, img, ppmmx=1, ppmmy=1, topleftx=-0.052, toplefty=0.469):
        self.img = img
        self.ppmmx = ppmmx
        self.ppmmy = ppmmy
        self.topleftx = topleftx
        self.toplefty = toplefty
    
    def getcommands(self):
        ''' Divides img into Paths and then Paths into Commands '''
        ns = {"svg": "http://www.w3.org/2000/svg"}
        root = ET.fromstring(self.img)
        commands = []
        init_pos = Path("M 0 0", self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
        commands.append(init_pos.pathtolist()[0])

        for path in root.findall("svg:path", ns):
            parsed_path = Path(path.attrib["d"], self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
            path_cmds = parsed_path.pathtolist()
            for cmd in path_cmds:
                commands.append(cmd)

        for circle in root.findall("svg:circle", ns):
            circle_init = Path(f"M {int(circle.attrib["cx"])+int(circle.attrib["r"])} {int(circle.attrib["cy"])}", self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
            commands.append(circle_init.pathtolist()[0])
            circle_path = Command("Ell", [], self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
            r = circle_path.normalizetophys(int(circle.attrib["r"]), int(circle.attrib["r"]))[0]
            cx, cy = circle_path.coordtophys(int(circle.attrib["cx"]), int(circle.attrib["cy"]))
            circle_path.coords = [r, r, cx, cy] 
            commands.append(circle_path)

        for ellipse in root.findall("svg:ellipse", ns):
            ellipse_init = Path(f"M {int(ellipse.attrib["cx"])+int(ellipse.attrib["rx"])} {int(ellipse.attrib["cy"])}", self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
            commands.append(ellipse_init.pathtolist()[0])
            ellipse_path = Command("Ell", [], self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
            rx, ry = ellipse_path.normalizetophys(int(ellipse.attrib["rx"]), int(ellipse.attrib["ry"]))
            cx, cy = ellipse_path.coordtophys(int(ellipse.attrib["cx"]), int(ellipse.attrib["cy"]))
            ellipse_path.coords = [rx, ry, cx, cy] 
            commands.append(ellipse_path)

        for rect in root.findall("svg:rect", ns):
            rect_path = Path(f"M {int(rect.attrib["x"])} {int(rect.attrib["y"])}"+
                             f"L {int(rect.attrib["x"])+int(rect.attrib["width"])} {int(rect.attrib["y"])}"+
                             f"L {int(rect.attrib["x"])+int(rect.attrib["width"])} {int(rect.attrib["y"])+int(rect.attrib["height"])}"+
                             f"L {int(rect.attrib["x"])} {int(rect.attrib["y"])+int(rect.attrib["height"])}"+
                             f"L {int(rect.attrib["x"])} {int(rect.attrib["y"])}",  self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
            for cmd in rect_path.pathtolist():
                commands.append(cmd)

        return commands
    

class Path:
    def __init__(self, d, ppmmx, ppmmy, topleftx, toplefty):
        self.d = d
        self.ppmmx = ppmmx
        self.ppmmy = ppmmy
        self.topleftx = topleftx
        self.toplefty = toplefty

    def pathtolist(self):
        ''' Converts the text path to a list of Command elements '''
        pathstrlst = [] # A path of strings which are split into letters and numbers
        pathcmds = [] # result
        # Seperate path by spaces
        spltspaces = self.d.split()
        # Sometimes there is no space between a letter and the beginning of a number
        for elem in spltspaces:
            idx = 0
            lastsplit = 0
            for idx in range(1, len(elem)):
                if not ((elem[idx].isalpha() and elem[idx-1].isalpha()) or (elem[idx].isdigit() and elem[idx-1].isdigit())):
                    pathstrlst.append(elem[lastsplit:idx])
                    lastsplit = idx
            if not (lastsplit == len(elem)):
                pathstrlst.append(elem[lastsplit:len(elem)])
        # At this point pathstrlst is a list where each element is a string which is either a string or a number
        # Now convert each number from str to num and 
        pathelemlst = [int(elem) if elem.isdigit() else elem for elem in pathstrlst]
        # Convert the path elements into commands
        currcmd = None
        for elem in pathelemlst:
            if isinstance(elem, str):
                if not (currcmd == None):
                    pathcmds.append(currcmd)
                # A new element is beginning
                currcmd = Command(elem, [], self.ppmmx, self.ppmmy, self.topleftx, self.toplefty)
                currx = None
                curry = None
            elif currx == None:
                currx = elem
            elif curry == None:
                curry = elem
                # Convert the x,y coordinates to physical coordinates and add to command
                physx, physy = currcmd.coordtophys(currx, curry)
                currcmd.coords.append(physx)
                currcmd.coords.append(physy)
                currx = None
                curry = None
            else:
                print("# Possible Error: SVG not formatted correctly.")
        pathcmds.append(currcmd)
        return pathcmds
    
    
class Command:
    def __init__(self, type, coords, ppmmx, ppmmy, topleftx, toplefty):
        self.type = type
        self.ppmmx = ppmmx
        self.ppmmy = ppmmy
        self.topleftx = topleftx
        self.toplefty = toplefty
        self.coords = coords # physical list of scalars

    def coordtophys(self, x, y):
        ''' Convert x, y to absolute physical coordinates '''
        physx = self.topleftx + x/self.ppmmx*0.001 # in mm
        physy = self.toplefty - y/self.ppmmy*0.001 # in mm
        return (physx, physy)
    
    def normalizetophys(self, x, y):
        ''' Convert x, y to relative physical coordinates '''
        physx = x/self.ppmmx*0.001 # in mm
        physy = y/self.ppmmy*0.001 # in mm
        return (physx, physy)

    def gettype(self):
        return self.type

    def getcoords(self):
        return self.coords
    
    def __repr__(self):
        return f"<Type: {self.type}, Coords: [" + ", ".join([f"{x:.4f}" for x in self.coords]) + "]>"


def main(argv):
    pass

if __name__ == '__main__':
    main(sys.argv)