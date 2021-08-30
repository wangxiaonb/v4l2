#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "v4l2.hpp"

typedef struct
{
    PyObject_HEAD;
    void *start;
    size_t length;
} PyFrameObject;

PyObject *py_add(PyObject *self, PyObject *args)
{
    int a, b;
    PyArg_ParseTuple(args, "ii", &a, &b);
    int result = add(a, b);
    return PyLong_FromLong(result);
}

char b[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};

PyObject *py_test(PyObject *self, PyObject *args)
{
    b[0]++;
    PyObject *object = PyMemoryView_FromMemory(b, sizeof(b), PyBUF_READ);
    printf("PyMemoryView_FromMemory: %x\n",(object));
    return object;
}

PyObject *py_test2(PyObject *self, PyObject *args)
{
    b[1]++;
    PyObject *object = PyMemoryView_FromMemory(b, sizeof(b), PyBUF_READ);
    printf("PyMemoryView_FromMemory: %x\n",(object));
    PyObject *result = PyByteArray_FromObject(object);
    printf("PyByteArray_FromObject : %x\n",(result));
    return result;
}

// void *open(const char *dev_name);
// void close(void *handle);
// void start(void *handle);
// void stop(void *handle);
// struct buffer read(void *handle);

PyObject *py_open(PyObject *self, PyObject *args)
{
    char *dev;
    PyArg_ParseTuple(args, "s", &dev);
    void *handle = open(dev);
    unsigned long result = (unsigned long)handle;
    printf("py_open %lu\n", result);
    return PyLong_FromUnsignedLong(result);
}

PyObject *py_close(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    close((void *)handle);
    printf("py_close %lu\n", handle);
    return Py_None;
}

PyObject *py_start(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    start((void *)handle);
    printf("py_start %lu\n", handle);
    return Py_None;
}

PyObject *py_stop(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    stop((void *)handle);
    printf("py_stop %lu\n", handle);
    return Py_None;
}

PyObject *py_read(PyObject *self, PyObject *args)
{
    struct buffer frame;
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    frame = read((void *)handle);
    PyObject *object = PyMemoryView_FromMemory((char*)frame.start, frame.length, PyBUF_READ);
    PyObject *result = PyByteArray_FromObject(object);
    return result;
}

static PyMethodDef py_methods[] = {
    // The first property is the name exposed to Python, fast_tanh, the second is the C++
    // function name that contains the implementation.
    {"open", (PyCFunction)py_open, METH_VARARGS, nullptr},
    {"close", (PyCFunction)py_close, METH_VARARGS, nullptr},
    {"start", (PyCFunction)py_start, METH_VARARGS, nullptr},
    {"stop", (PyCFunction)py_stop, METH_VARARGS, nullptr},
    {"read", (PyCFunction)py_read, METH_VARARGS, nullptr},

    {"add", (PyCFunction)py_add, METH_VARARGS, nullptr},
    {"test", (PyCFunction)py_test, METH_VARARGS, nullptr},
    {"test2", (PyCFunction)py_test2, METH_VARARGS, nullptr},
    
    // Terminate the array with an object containing nulls.
    {nullptr, nullptr, 0, nullptr}};

static PyModuleDef py_module = {
    PyModuleDef_HEAD_INIT,
    "v4l2",               // Module name to use with Python import statements
    "v4l2 python driver", // Module description
    0,
    py_methods // Structure that defines the methods of the module
};

PyMODINIT_FUNC PyInit_v4l2()
{
    return PyModule_Create(&py_module);
}
