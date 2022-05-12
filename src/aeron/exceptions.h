#ifndef AERON_PYTHON_EXCEPTIONS_H
#define AERON_PYTHON_EXCEPTIONS_H

#define PY_SSIZE_T_CLEAN

#include <Python.h>

static PyObject *AeronError;
static PyObject *AeronPublicationError;
static PyObject *AeronPublicationNotConnectedError;
static PyObject *AeronPublicationBackPressuredError;
static PyObject *AeronPublicationAdminActionError;
static PyObject *AeronPublicationClosedError;
static PyObject *AeronPublicationMaxPositionExceededError;

static int exceptions_init(PyObject *m);

#endif //AERON_PYTHON_EXCEPTIONS_H
