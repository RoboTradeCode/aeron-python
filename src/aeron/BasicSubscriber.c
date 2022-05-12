#include "BasicSubscriber.h"

typedef struct
{
    PyObject_HEAD
    PyObject *handler;
    aeron_context_t *context;
    aeron_t *aeron;
    aeron_async_add_publication_t *async;
    aeron_subscription_t *subscription;
    aeron_fragment_assembler_t *fragment_assembler;
} BasicSubscriberObject;

void poll_handler(void *clientd, const uint8_t *buffer, size_t Py_UNUSED(length), aeron_header_t *Py_UNUSED(header))
{
    BasicSubscriberObject *subscriber = (BasicSubscriberObject *) clientd;

    PyObject *arglist = Py_BuildValue("(s)", buffer);
    PyObject *result = PyObject_CallObject(subscriber->handler, arglist);

    if (result == NULL)
        return;

    // TODO: Warning
//    PyErr_WarnEx(PyExc_RuntimeWarning, )

    Py_DECREF(arglist);
    Py_DECREF(result);
}

static int
BasicSubscriber_init(BasicSubscriberObject *self, PyObject *args, PyObject *kwds)
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

    if (aeron_async_add_subscription(
        &self->async,
        self->aeron,
        channel,
        (int) stream_id,
        NULL,
        NULL,
        NULL,
        NULL
    ) < 0) {
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

//static PyObject *BasicSubscriber_add_destination(BasicSubscriberObject *self, PyObject *args) {
//    const char *channel;
//
//    if (!PyArg_ParseTuple(args, "s", &channel))
//        return NULL;
//
//    int result = aeron_subscription_async_add_destination(&self->async, self->aeron, self->subscription, channel);
//    return PyLong_FromLong(result);
//}
//
//static PyObject *BasicSubscriber_remove_destination(BasicSubscriberObject *self, PyObject *args) {
//    const char *channel;
//
//    if (!PyArg_ParseTuple(args, "s", &channel))
//        return NULL;
//
//    int result = aeron_subscription_async_remove_destination(&self->async, self->aeron, self->subscription, channel);
//    return PyLong_FromLong(result);
//}

static PyObject *
BasicSubscriber_poll(BasicSubscriberObject *self, PyObject *Py_UNUSED(args))
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

    return PyLong_FromLong(fragments_read);
}

static PyObject *
BasicSubscriber_close(BasicSubscriberObject *self, PyObject *Py_UNUSED(ignored))
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

static PyMethodDef BasicSubscriber_methods[] = {
    {"poll", (PyCFunction) BasicSubscriber_poll, METH_NOARGS,
        "Poll the images under the subscription for available message fragments"
    },
    {"close", (PyCFunction) BasicSubscriber_close, METH_NOARGS,
        "Close the subscription"
    },
    {NULL}
};

static PyTypeObject BasicSubscriberType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "aeron.BasicSubscriber",
    .tp_doc = PyDoc_STR("A simple subscriber that receives messages"),
    .tp_basicsize = sizeof(BasicSubscriberObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) BasicSubscriber_init,
    .tp_methods = BasicSubscriber_methods,
};
