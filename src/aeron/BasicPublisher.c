#include "BasicPublisher.h"

typedef struct
{
    PyObject_HEAD
    aeron_context_t *context;
    aeron_t *aeron;
    aeron_publication_t *publication;
} BasicPublisherObject;

static int
BasicPublisher_init(BasicPublisherObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"channel", "stream_id", NULL};
    aeron_async_add_publication_t *async = NULL;
    const char *channel = DEFAULT_CHANNEL;
    int stream_id = DEFAULT_STREAM_ID;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|si", kwlist, &channel, &stream_id)) {
        return -1;
    }

    if (aeron_context_init(&self->context) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return -1;
    }

    if (aeron_init(&self->aeron, self->context) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return -1;
    }

    if (aeron_start(self->aeron) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return -1;
    }

    if (aeron_async_add_publication(&async, self->aeron, channel, stream_id) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return -1;
    }

    Py_BEGIN_ALLOW_THREADS
        while (NULL == self->publication) {
            if (aeron_async_add_publication_poll(&self->publication, async) < 0) {
                Py_BLOCK_THREADS
                PyErr_SetString(AeronError, aeron_errmsg());
                return -1;
            }

            sched_yield();
        }
    Py_END_ALLOW_THREADS

    return 0;
}

static PyObject *
BasicPublisher_offer(BasicPublisherObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"message", NULL};
    const char *message;
    Py_ssize_t message_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#", kwlist, &message, &message_length)) {
        return NULL;
    }

    int64_t result = aeron_publication_offer(self->publication, (const uint8_t *) message, message_length, NULL, NULL);

    if (AERON_PUBLICATION_NOT_CONNECTED == result) {
        PyErr_SetString(AeronPublicationNotConnectedError, aeron_errmsg());
        return NULL;
    } else if (AERON_PUBLICATION_BACK_PRESSURED == result) {
        PyErr_SetString(AeronPublicationBackPressuredError, aeron_errmsg());
        return NULL;
    } else if (AERON_PUBLICATION_ADMIN_ACTION == result) {
        PyErr_SetString(AeronPublicationAdminActionError, aeron_errmsg());
        return NULL;
    } else if (AERON_PUBLICATION_CLOSED == result) {
        PyErr_SetString(AeronPublicationClosedError, aeron_errmsg());
        return NULL;
    } else if (AERON_PUBLICATION_MAX_POSITION_EXCEEDED == result) {
        PyErr_SetString(AeronPublicationMaxPositionExceededError, aeron_errmsg());
        return NULL;
    } else if (AERON_PUBLICATION_ERROR == result) {
        PyErr_SetString(AeronPublicationError, aeron_errmsg());
        return NULL;
    }

    // TODO: Create property for calling aeron_publication_is_connected

    return PyLong_FromLong(result);
}

static PyObject *
BasicPublisher_close(BasicPublisherObject *self, PyObject *Py_UNUSED(ignored))
{
    if (aeron_publication_close(self->publication, NULL, NULL) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    if (aeron_close(self->aeron) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    if (aeron_context_close(self->context) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyMethodDef BasicPublisher_methods[] = {
    {"offer", (PyCFunction) BasicPublisher_offer, METH_VARARGS | METH_KEYWORDS,
        "Publish the message"
    },
    {"close", (PyCFunction) BasicPublisher_close, METH_NOARGS,
        "Close the publication"
    },
    {NULL},
};

static PyTypeObject BasicPublisherType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "aeron.BasicPublisher",
    .tp_doc = PyDoc_STR("A simple publisher that sends messages"),
    .tp_basicsize = sizeof(BasicPublisherObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) BasicPublisher_init,
    .tp_methods = BasicPublisher_methods,
};
