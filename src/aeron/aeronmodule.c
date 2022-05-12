#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "exceptions.h"
#include "BasicPublisher.h"
#include "BasicSubscriber.h"

static PyModuleDef aeronmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "aeron",
    .m_doc = "Efficient reliable UDP unicast, UDP multicast, and IPC message transport",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_aeron(void)
{
    PyObject *m;
    if (PyType_Ready(&BasicPublisherType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&aeronmodule);
    if (m == NULL) {
        return NULL;
    }

    if (exceptions_init(m) < 0)
    {
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&BasicPublisherType);
    if (PyModule_AddObject(m, "BasicPublisher", (PyObject *) &BasicPublisherType) < 0) {
        Py_DECREF(&BasicPublisherType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&BasicSubscriberType);
    if (PyModule_AddObject(m, "BasicSubscriber", (PyObject *) &BasicSubscriberType) < 0) {
        Py_DECREF(&BasicSubscriberType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
