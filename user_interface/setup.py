from setuptools import Extension, setup

#This file's purpose is to create the Python module

module = Extension("DrawBotRunner", sources=['launchmodule.c',"launch.c"])
setup(name='DrawBotRunner',
     version='1.0',
     description='',
     ext_modules=[module])