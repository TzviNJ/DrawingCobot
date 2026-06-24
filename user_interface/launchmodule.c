# define PY_SSIZE_T_CLEAN
# include <Python.h>
# include "launch.h"

/* This file's purpose is to define a Python module with the functionalities of the C program. */

static PyObject* py_launch_seq(PyObject* self, PyObject* args) {
    /* Defines the functionality of launch_seq */
    int launch_type;
    int progmode;
    char* img_path;
    double img_res;
    if (!PyArg_ParseTuple(args, "iisd", &launch_type, &progmode, &img_path, &img_res)) {
        return NULL;
    }
    return Py_BuildValue("i", launch_seq(launch_type, progmode, img_path, img_res));
}

static PyObject* py_kill_launch(PyObject* self, PyObject* args) {
    /* Defines the functionality of kill_launch */
    int launch_type;
    int timeout;
    if (!PyArg_ParseTuple(args, "ii", &launch_type, &timeout)) {
        return NULL;
    }
    return Py_BuildValue("i", kill_launch(launch_type, timeout));
}

static PyObject* py_cleanup_launches(PyObject* self, PyObject* args) {
    /* Defines the functionality of cleanup_launches */
    cleanup_launches();
    return Py_BuildValue("i", 1);
}

/* A struct that defines the names of the functions in Python */
static PyMethodDef RunnerMethods[] = {
    {"launch_seq",                  
      (PyCFunction) py_launch_seq, 
      METH_VARARGS,          
      PyDoc_STR("")}, 
    {"kill_launch",                  
    (PyCFunction) py_kill_launch, 
    METH_VARARGS,          
    PyDoc_STR("")}, 
    {"cleanup_launches",                  
        (PyCFunction) py_cleanup_launches, 
        METH_VARARGS,          
        PyDoc_STR("")}, 
    
    {NULL, NULL, 0, NULL}     
};

/* A struct that defines the modules name and methods */
static struct PyModuleDef runner_module = {
    PyModuleDef_HEAD_INIT,
    "DrawBotRunner", /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    RunnerMethods /* the PyMethodDef array from before containing the methods of the extension */
};

PyMODINIT_FUNC PyInit_DrawBotRunner(void)
{
    /* This function initiates the module/ */
    PyObject *m;
    m = PyModule_Create(&runner_module);
    if (!m) {
        return NULL;
    }
    return m;
}
