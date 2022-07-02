#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <aeron/aeronc.h>

#define DEFAULT_CHANNEL "aeron:udp?endpoint=localhost:20121"
#define DEFAULT_STREAM_ID (1001)
#define DEFAULT_FRAGMENT_COUNT_LIMIT (1)

static PyObject *AeronError;
static PyObject *AeronPublicationError;
static PyObject *AeronPublicationNotConnectedError;
static PyObject *AeronPublicationBackPressuredError;
static PyObject *AeronPublicationAdminActionError;
static PyObject *AeronPublicationClosedError;
static PyObject *AeronPublicationMaxPositionExceededError;

typedef struct
{
    PyObject_HEAD
    aeron_context_t *context;
    aeron_t *aeron;
    aeron_publication_t *publication;
} PublisherObject;

static int
Publisher_init(PublisherObject *self, PyObject *args, PyObject *kwds)
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
Publisher_offer(PublisherObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"message", NULL};
    const char *message;
    Py_ssize_t message_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#", kwlist, &message, &message_length)) {
        return NULL;
    }

    int64_t result = aeron_publication_offer(
        self->publication,
        (const uint8_t *) message,
        message_length,
        NULL,
        NULL
    );

    if (AERON_PUBLICATION_NOT_CONNECTED == result) {
        PyErr_SetString(
            AeronPublicationNotConnectedError,
            "Offer failed because publisher is not connected to a subscriber"
        );
        return NULL;
    } else if (AERON_PUBLICATION_BACK_PRESSURED == result) {
        PyErr_SetString(
            AeronPublicationBackPressuredError,
            "Offer failed due to back pressure"
        );
        return NULL;
    } else if (AERON_PUBLICATION_ADMIN_ACTION == result) {
        PyErr_SetString(
            AeronPublicationAdminActionError,
            "Offer failed because of an administration action in the system"
        );
        return NULL;
    } else if (AERON_PUBLICATION_CLOSED == result) {
        PyErr_SetString(
            AeronPublicationClosedError,
            "Offer failed because publication is closed"
        );
        return NULL;
    } else if (AERON_PUBLICATION_MAX_POSITION_EXCEEDED == result) {
        PyErr_SetString(
            AeronPublicationMaxPositionExceededError,
            "Offer failed due to reaching the maximum position"
        );
        return NULL;
    } else if (AERON_PUBLICATION_ERROR == result) {
        char error_string[256];
        snprintf(error_string, 256, "Offer failed due to unknown reason %ld", result);
        PyErr_SetString(AeronPublicationError, error_string);
        return NULL;
    }

    return PyLong_FromLong(result);
}

static PyObject *
Publisher_close(PublisherObject *self, PyObject *Py_UNUSED(ignored))
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

static PyMethodDef Publisher_methods[] = {
    {"offer", (PyCFunction) Publisher_offer, METH_VARARGS | METH_KEYWORDS,
        "Publish the message"
    },
    {"close", (PyCFunction) Publisher_close, METH_NOARGS,
        "Close the publication"
    },
    {NULL},
};

static PyTypeObject PublisherType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "aeron.Publisher",
    .tp_doc = PyDoc_STR("A simple publisher that sends messages"),
    .tp_basicsize = sizeof(PublisherObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Publisher_init,
    .tp_methods = Publisher_methods,
};

typedef struct
{
    PyObject_HEAD
    PyObject *handler;
    aeron_context_t *context;
    aeron_t *aeron;
    aeron_async_add_publication_t *async;
    aeron_subscription_t *subscription;
    aeron_fragment_assembler_t *fragment_assembler;
} SubscriberObject;

static void
Subscriber_dealloc(SubscriberObject *self)
{
    Py_XDECREF(self->handler);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

bool HandlerErr_Occurred = false;

void
poll_handler(void *clientd, const uint8_t *buffer, size_t length, aeron_header_t *Py_UNUSED(header))
{
    SubscriberObject *subscriber = (SubscriberObject *) clientd;

    PyObject *arglist = Py_BuildValue("(s#)", buffer, length);
    if (arglist == NULL) {
        return;
    }

    PyObject *result = PyObject_CallObject(subscriber->handler, arglist);
    Py_DECREF(arglist);
    if (result == NULL) {
        HandlerErr_Occurred = true;
        return;
    }

    Py_DECREF(result);
}

static int
Subscriber_init(SubscriberObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"handler", "channel", "stream_id", NULL};
    PyObject *handler = NULL;
    const char *channel = DEFAULT_CHANNEL;
    int stream_id = DEFAULT_STREAM_ID;

    if (PyArg_ParseTupleAndKeywords(args, kwds, "O|si", kwlist, &handler, &channel, &stream_id)) {
        if (!PyCallable_Check(handler)) {
            PyErr_SetString(PyExc_TypeError, "handler must be callable");
            return -1;
        }
        PyObject *tmp = self->handler;
        Py_INCREF(handler);
        self->handler = handler;
        Py_XDECREF(tmp);
    } else {
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

    int result = aeron_async_add_subscription(
        &self->async,
        self->aeron,
        channel,
        (int) stream_id,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (result < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return -1;
    }

    Py_BEGIN_ALLOW_THREADS
        while (NULL == self->subscription) {
            if (aeron_async_add_subscription_poll(&self->subscription, self->async) < 0) {
                Py_BLOCK_THREADS
                PyErr_SetString(AeronError, aeron_errmsg());
                return -1;
            }

            sched_yield();
        }
    Py_END_ALLOW_THREADS

    if (aeron_fragment_assembler_create(&self->fragment_assembler, poll_handler, self) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return -1;
    }

    return 0;
}

static PyObject *
Subscriber_add_destination(SubscriberObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"channel", NULL};
    const char *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &channel)) {
        return NULL;
    }

    int result = aeron_subscription_async_add_destination(
        &self->async,
        self->aeron,
        self->subscription,
        channel
    );

    if (result < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
Subscriber_remove_destination(SubscriberObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"channel", NULL};
    const char *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &channel)) {
        return NULL;
    }

    int result = aeron_subscription_async_remove_destination(
        &self->async,
        self->aeron,
        self->subscription,
        channel
    );

    if (result < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
Subscriber_poll(SubscriberObject *self, PyObject *Py_UNUSED(args))
{
    int fragments_read = aeron_subscription_poll(
        self->subscription,
        aeron_fragment_assembler_handler,
        self->fragment_assembler,
        DEFAULT_FRAGMENT_COUNT_LIMIT
    );

    if (fragments_read < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    if (HandlerErr_Occurred) {
        HandlerErr_Occurred = false;
        return NULL;
    }

    return PyLong_FromLong(fragments_read);
}

static PyObject *
Subscriber_close(SubscriberObject *self, PyObject *Py_UNUSED(ignored))
{
    if (aeron_subscription_close(self->subscription, NULL, NULL) < 0) {
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

    if (aeron_fragment_assembler_delete(self->fragment_assembler) < 0) {
        PyErr_SetString(AeronError, aeron_errmsg());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyMethodDef Subscriber_methods[] = {
    {"add_destination",    (PyCFunction) Subscriber_add_destination,    METH_VARARGS | METH_KEYWORDS,
        "Add a destination manually to a multi-destination-subscription"
    },
    {"remove_destination", (PyCFunction) Subscriber_remove_destination, METH_VARARGS | METH_KEYWORDS,
        "Remove a destination manually from a multi-destination-subscription"
    },
    {"poll",               (PyCFunction) Subscriber_poll,               METH_NOARGS,
        "Poll the images under the subscription for available message fragments"
    },
    {"close",              (PyCFunction) Subscriber_close,              METH_NOARGS,
        "Close the subscription"
    },
    {NULL}
};

static PyTypeObject SubscriberType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "aeron.Subscriber",
    .tp_doc = PyDoc_STR("A simple subscriber that receives messages"),
    .tp_basicsize = sizeof(SubscriberObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Subscriber_init,
    .tp_dealloc = (destructor) Subscriber_dealloc,
    .tp_methods = Subscriber_methods,
};

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

static PyModuleDef aeronmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "aeron",
    .m_doc = "Efficient reliable UDP unicast, UDP multicast, and IPC message transport",
    .m_size = -1,
};

__attribute__((unused)) PyMODINIT_FUNC
PyInit_aeron(void)
{
    PyObject *m;

    if (PyType_Ready(&PublisherType) < 0) {
        return NULL;
    }

    if (PyType_Ready(&SubscriberType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&aeronmodule);
    if (m == NULL) {
        return NULL;
    }

    if (exceptions_init(m) < 0) {
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&PublisherType);
    if (PyModule_AddObject(m, "Publisher", (PyObject *) &PublisherType) < 0) {
        Py_DECREF(&PublisherType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&SubscriberType);
    if (PyModule_AddObject(m, "Subscriber", (PyObject *) &SubscriberType) < 0) {
        Py_DECREF(&SubscriberType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
