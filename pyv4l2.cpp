#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "v4l2.hpp"

typedef struct
{
    PyObject_HEAD;
    void *start;
    size_t length;
} PyFrameObject;

char b[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};

PyObject *py_test(PyObject *self, PyObject *args)
{
    b[0]++;
    PyObject *object = PyMemoryView_FromMemory(b, sizeof(b), PyBUF_READ);
    return object;
}

PyObject *py_test2(PyObject *self, PyObject *args)
{
    b[1]++;
    PyObject *object = PyMemoryView_FromMemory(b, sizeof(b), PyBUF_READ);
    PyObject *result = PyByteArray_FromObject(object);
    return result;
}

PyObject *py_echo(PyObject *self, PyObject *args)
{
    const char *what;
    PyArg_ParseTuple(args, "s", &what);
    printf("Python said: %s", what);
    return Py_None;
}

PyObject *py_open(PyObject *self, PyObject *args)
{
    const char *dev;
    PyArg_ParseTuple(args, "s", &dev);
    printf("camera dev: %s\n", dev);
    void *handle = open(dev);
    unsigned long result = (unsigned long)handle;
    // printf("py_open %lu\n", result);
    return PyLong_FromUnsignedLong(result);
}

PyObject *py_open2(PyObject *self, PyObject *args)
{
    const char *dev;
    int width;
    int height;
    const char *color;
    PyArg_ParseTuple(args, "siis", &dev, &width, &height, &color);
    printf("camera dev: %s\n", dev);
    printf("%d x %d, %s\n", width, height, color);
    void *handle = open(dev, width, height, color);
    unsigned long result = (unsigned long)handle;
    // printf("py_open %lu\n", result);
    return PyLong_FromUnsignedLong(result);
}

PyObject *py_close(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    close((void *)handle);
    // printf("py_close %lu\n", handle);
    return Py_None;
}

PyObject *py_start(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    start((void *)handle);
    // printf("py_start %lu\n", handle);
    return Py_None;
}

PyObject *py_stop(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    stop((void *)handle);
    // printf("py_stop %lu\n", handle);
    return Py_None;
}

PyObject *py_read(PyObject *self, PyObject *args)
{
    struct buffer frame;
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    frame = read((void *)handle);
    PyObject *object = PyMemoryView_FromMemory((char *)frame.start, frame.length, PyBUF_READ);
    PyObject *result = PyByteArray_FromObject(object);
    return result;
}

PyObject *py_setformat(PyObject *self, PyObject *args)
{
    unsigned long handle;
    int width;
    int height;
    const char *color;
    PyArg_ParseTuple(args, "liis", &handle, &width, &height, &color);
    printf("set resulotion: %d x %d\n", width, height);
    printf("set color: %s\n", color);
    setformat((void *)handle, width, height, color);
    return Py_None;
}

PyObject *py_setcontrol(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __u32 id;
    __s32 value;
    PyArg_ParseTuple(args, "lii", &handle, &id, &value);
    // printf("set control,id: %d  value: %d\n", id, value);
    setcontrol((void *)handle, id, value);
    return Py_None;
}

PyObject *py_getcontrol(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __u32 id;
    __s32 value;
    PyArg_ParseTuple(args, "li", &handle, &id);
    value = getcontrol((void *)handle, id);
    // printf("get control,id: %d  value: %d\n", id, value);
    return PyLong_FromLong(value);
}

void set_sensor_reg(void *handle, __u8 reg, __u16 data);
__u16 get_sensor_reg(void *handle, __u8 reg);
void snapshot(void *handle);

PyObject *py_get_sensor_reg(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __u8 reg;
    __u16 value;
    PyArg_ParseTuple(args, "lb", &handle, &reg);
    value = get_sensor_reg((void *)handle, reg);
    printf("get_sensor_reg: %d\n", value);
    return PyLong_FromLong(value);
}

PyObject *py_set_sensor_reg(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __u8 reg;
    __s16 value;
    PyArg_ParseTuple(args, "lbh", &handle, &reg, &value);
    set_sensor_reg((void *)handle, reg, value);
    printf("set_sensor_reg: %02x, %04x\n", reg, value);
    return Py_None;
}

PyObject *py_snapshot(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    snapshot((void *)handle);
    // printf("snapshot\n");
    return Py_None;
}

static PyMethodDef py_methods[] = {
    // The first property is the name exposed to Python, fast_tanh, the second is the C++
    // function name that contains the implementation.
    {"open", (PyCFunction)py_open, METH_VARARGS, nullptr},
    {"open2", (PyCFunction)py_open2, METH_VARARGS, nullptr},
    {"close", (PyCFunction)py_close, METH_VARARGS, nullptr},
    {"start", (PyCFunction)py_start, METH_VARARGS, nullptr},
    {"stop", (PyCFunction)py_stop, METH_VARARGS, nullptr},
    {"read", (PyCFunction)py_read, METH_VARARGS, nullptr},
    {"setformat", (PyCFunction)py_setformat, METH_VARARGS, nullptr},
    {"setcontrol", (PyCFunction)py_setcontrol, METH_VARARGS, nullptr},
    {"getcontrol", (PyCFunction)py_getcontrol, METH_VARARGS, nullptr},
    {"get_sensor_reg", (PyCFunction)py_get_sensor_reg, METH_VARARGS, nullptr},
    {"set_sensor_reg", (PyCFunction)py_set_sensor_reg, METH_VARARGS, nullptr},
    {"snapshot", (PyCFunction)py_snapshot, METH_VARARGS, nullptr},

    {"echo", (PyCFunction)py_echo, METH_VARARGS, nullptr},
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
