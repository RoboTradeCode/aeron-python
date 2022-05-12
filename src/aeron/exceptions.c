#include "exceptions.h"

static int exceptions_init(PyObject *m)
{
    AeronError = PyErr_NewException("aeron.AeronError", NULL, NULL);
    Py_XINCREF(AeronError);
    if (PyModule_AddObject(m, "AeronError", AeronError) < 0) {
        Py_XDECREF(AeronError);
        Py_CLEAR(AeronError);
        return -1;
    }

    AeronPublicationError = PyErr_NewExceptionWithDoc(
        "aeron.AeronPublicationError",
        PyDoc_STR("An error has occurred. Such as a bad argument"),
        AeronError,
        NULL
    );
    Py_XINCREF(AeronPublicationError);
    if (PyModule_AddObject(m, "AeronPublicationError", AeronPublicationError) < 0) {
        Py_XDECREF(AeronPublicationError);
        Py_CLEAR(AeronPublicationError);
        return -1;
    }

    AeronPublicationNotConnectedError = PyErr_NewExceptionWithDoc(
        "aeron.AeronPublicationNotConnectedError",
        PyDoc_STR("The publication is not connected to a subscriber, "
                  "this can be an intermittent state as subscribers come and go"),
        AeronPublicationError,
        NULL
    );
    Py_XINCREF(AeronPublicationNotConnectedError);
    if (PyModule_AddObject(m, "AeronPublicationNotConnectedError", AeronPublicationNotConnectedError) < 0) {
        Py_XDECREF(AeronPublicationNotConnectedError);
        Py_CLEAR(AeronPublicationNotConnectedError);
        return -1;
    }

    AeronPublicationBackPressuredError = PyErr_NewExceptionWithDoc(
        "aeron.AeronPublicationBackPressuredError",
        PyDoc_STR("The offer failed due to back pressure from the subscribers preventing further transmission"),
        AeronPublicationError,
        NULL
    );
    Py_XINCREF(AeronPublicationBackPressuredError);
    if (PyModule_AddObject(m, "AeronPublicationBackPressuredError", AeronPublicationBackPressuredError) < 0) {
        Py_XDECREF(AeronPublicationBackPressuredError);
        Py_CLEAR(AeronPublicationBackPressuredError);
        return -1;
    }

    AeronPublicationAdminActionError = PyErr_NewExceptionWithDoc(
        "aeron.AeronPublicationAdminActionError",
        PyDoc_STR("The offer failed due to an administration action and should be retried.\n"
                  "The action is an operation such as log rotation "
                  "which is likely to have succeeded by the next retry attempt"),
        AeronPublicationError,
        NULL
    );
    Py_XINCREF(AeronPublicationAdminActionError);
    if (PyModule_AddObject(m, "AeronPublicationAdminActionError", AeronPublicationAdminActionError) < 0) {
        Py_XDECREF(AeronPublicationAdminActionError);
        Py_CLEAR(AeronPublicationAdminActionError);
        return -1;
    }

    AeronPublicationClosedError = PyErr_NewExceptionWithDoc(
        "aeron.AeronPublicationClosedError",
        PyDoc_STR("The publication has been closed and should no longer be used"),
        AeronPublicationError,
        NULL
    );
    Py_XINCREF(AeronPublicationClosedError);
    if (PyModule_AddObject(m, "AeronPublicationClosedError", AeronPublicationClosedError) < 0) {
        Py_XDECREF(AeronPublicationClosedError);
        Py_CLEAR(AeronPublicationClosedError);
        return -1;
    }

    AeronPublicationMaxPositionExceededError = PyErr_NewExceptionWithDoc(
        "aeron.AeronPublicationMaxPositionExceededError",
        PyDoc_STR("The offer failed due to reaching the maximum position "
                  "of the stream given term buffer length times the total possible number of terms.\n"
                  "\n"
                  "If this happens then the publication should be closed and a new one added. "
                  "To make it less likely to happen then increase the term buffer length"),
        AeronPublicationError,
        NULL
    );
    Py_XINCREF(AeronPublicationMaxPositionExceededError);
    if (PyModule_AddObject(
        m,
        "AeronPublicationMaxPositionExceededError",
        AeronPublicationMaxPositionExceededError
    ) < 0) {
        Py_XDECREF(AeronPublicationMaxPositionExceededError);
        Py_CLEAR(AeronPublicationMaxPositionExceededError);
        return -1;
    }

    return 0;
}
