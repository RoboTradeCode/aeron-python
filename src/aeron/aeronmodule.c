#define PY_SSIZE_T_CLEAN

#include <stdio.h>
#include <Python.h>
#include <aeronc.h>

typedef struct
{
    PyObject_HEAD
    PyObject* handler;
    aeron_context_t* context;
    aeron_t* aeron;
    aeron_async_add_publication_t* async;
    aeron_subscription_t* subscription;
    aeron_fragment_assembler_t* fragment_assembler;
    int fragment_limit;
} SubscriberObject;

static void Subscriber_dealloc(SubscriberObject* self)
{
    aeron_subscription_close(self->subscription, NULL, NULL);
    aeron_close(self->aeron);
    aeron_context_close(self->context);
    aeron_fragment_assembler_delete(self->fragment_assembler);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Subscriber_new(PyTypeObject* type, PyObject* Py_UNUSED(args), PyObject* Py_UNUSED(kwds))
{
    SubscriberObject* self;
    self = (SubscriberObject*)type->tp_alloc(type, 0);
    return (PyObject*)self;
}

void poll_handler(void* clientd, const uint8_t* buffer, size_t Py_UNUSED(length), aeron_header_t* Py_UNUSED(header))
{
    SubscriberObject* subscriber = (SubscriberObject*)clientd;

    PyObject* arglist = Py_BuildValue("(s)", buffer);
    PyObject* result = PyObject_CallObject(subscriber->handler, arglist);

    Py_DECREF(arglist);
    Py_DECREF(result);
}

static int Subscriber_init(SubscriberObject* self, PyObject* args, PyObject* Py_UNUSED(kwds))
{
    PyObject* handler;
    const char* channel = "aeron:udp?control-mode=manual";
    int stream_id = 1001;
    int fragment_limit = 10;

    if (PyArg_ParseTuple(args, "O|sii", &handler, &channel, &stream_id, &fragment_limit))
    {
        if (!PyCallable_Check(handler))
        {
            PyErr_SetString(PyExc_TypeError, "handler must be callable");
            return -1;
        }
        Py_XINCREF(handler);
        self->handler = handler;
    }
    else
    {
        return -1;
    }

    self->fragment_limit = fragment_limit;

    if (aeron_context_init(&self->context) < 0)
        printf("aeron_context_init: %s\n", aeron_errmsg());
    if (aeron_init(&self->aeron, self->context) < 0)
        printf("aeron_init: %s\n", aeron_errmsg());
    if (aeron_start(self->aeron) < 0)
        printf("aeron_start: %s\n", aeron_errmsg());
    if (aeron_async_add_subscription(&self->async, self->aeron, channel, (int)stream_id, NULL, NULL, NULL, NULL) < 0)
        printf("aeron_async_add_subscription: %s\n", aeron_errmsg());
    while (NULL == self->subscription)
        if (aeron_async_add_subscription_poll(&self->subscription, self->async) < 0)
            printf("aeron_async_add_subscription_poll: %s\n", aeron_errmsg());

    if (aeron_fragment_assembler_create(&self->fragment_assembler, poll_handler, self) < 0)
        printf("aeron_fragment_assembler_create: %s\n", aeron_errmsg());

    return 0;
}

static PyObject* Subscriber_add_destination(SubscriberObject* self, PyObject* args)
{
    const char* channel;

    if (!PyArg_ParseTuple(args, "s", &channel))
        return NULL;

    int result = aeron_subscription_async_add_destination(&self->async, self->aeron, self->subscription, channel);
    return PyLong_FromLong(result);
}

static PyObject* Subscriber_remove_destination(SubscriberObject* self, PyObject* args)
{
    const char* channel;

    if (!PyArg_ParseTuple(args, "s", &channel))
        return NULL;

    int result = aeron_subscription_async_remove_destination(&self->async, self->aeron, self->subscription, channel);
    return PyLong_FromLong(result);
}

static PyObject* Subscriber_poll(SubscriberObject* self, PyObject* Py_UNUSED(args))
{
    int fragments_read = aeron_subscription_poll(self->subscription, aeron_fragment_assembler_handler,
        self->fragment_assembler, self->fragment_limit);

    if (fragments_read < 0)
        printf("aeron_subscription_poll: %s\n", aeron_errmsg());

    return PyLong_FromLong(fragments_read);
}

static PyMethodDef Subscriber_methods[] = {
    { "add_destination", (PyCFunction)Subscriber_add_destination, METH_VARARGS,
        "Add a destination manually to a multi-destination-subscription"
    },
    { "remove_destination", (PyCFunction)Subscriber_remove_destination, METH_VARARGS,
        "Remove a destination manually from a multi-destination-subscription."
    },
    { "poll", (PyCFunction)Subscriber_poll, METH_NOARGS,
        "Poll the images under the subscription for available message fragments"
    },
    { NULL }
};

static PyTypeObject SubscriberType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "aeron.Subscriber",
    .tp_doc = "The client application which consumes messages from Publication Images captured by a Subscription",
    .tp_basicsize = sizeof(SubscriberObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Subscriber_new,
    .tp_init = (initproc)Subscriber_init,
    .tp_dealloc = (destructor)Subscriber_dealloc,
    .tp_methods = Subscriber_methods
};

typedef struct
{
    PyObject_HEAD
    aeron_context_t* context;
    aeron_t* aeron;
    aeron_publication_t* publication;
} PublisherObject;

static void Publisher_dealloc(PublisherObject* self)
{
    aeron_publication_close(self->publication, NULL, NULL);
    aeron_close(self->aeron);
    aeron_context_close(self->context);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Publisher_new(PyTypeObject* type, PyObject* Py_UNUSED(args), PyObject* Py_UNUSED(kwds))
{
    PublisherObject* self;
    self = (PublisherObject*)type->tp_alloc(type, 0);
    return (PyObject*)self;
}

static int Publisher_init(PublisherObject* self, PyObject* args, PyObject* Py_UNUSED(kwds))
{
    const char* channel = "aeron:udp?control-mode=manual";
    int stream_id = 1001;

    if (!PyArg_ParseTuple(args, "|si", &channel, &stream_id))
        return -1;

    aeron_async_add_publication_t* async = NULL;

    if (aeron_context_init(&self->context) < 0)
        fprintf(stderr, "aeron_context_init: %s\n", aeron_errmsg());
    if (aeron_init(&self->aeron, self->context) < 0)
        fprintf(stderr, "aeron_init: %s\n", aeron_errmsg());
    if (aeron_start(self->aeron) < 0)
        fprintf(stderr, "aeron_start: %s\n", aeron_errmsg());
    if (aeron_async_add_publication(&async, self->aeron, channel, stream_id) < 0)
        fprintf(stderr, "aeron_async_add_publication: %s\n", aeron_errmsg());
    while (NULL == self->publication)
        if (aeron_async_add_publication_poll(&self->publication, async) < 0)
            fprintf(stderr, "aeron_async_add_publication_poll: %s\n", aeron_errmsg());

    return 0;
}

static PyObject* Publisher_offer(PublisherObject* self, PyObject* args)
{
    const char* message;
    Py_ssize_t message_len;

    if (!PyArg_ParseTuple(args, "s#", &message, &message_len))
        return NULL;

    int64_t result = aeron_publication_offer(self->publication, (const uint8_t*)message, message_len, NULL, NULL);

    if (AERON_PUBLICATION_BACK_PRESSURED == result)
        printf("Offer failed due to back pressure\n");
    else if (AERON_PUBLICATION_NOT_CONNECTED == result)
        printf("Offer failed because publisher is not connected to a subscriber\n");
    else if (AERON_PUBLICATION_ADMIN_ACTION == result)
        printf("Offer failed because of an administration action in the system\n");
    else if (AERON_PUBLICATION_CLOSED == result)
        printf("Offer failed because publication is closed\n");
    else if (result <= 0)
        printf("Offer failed due to unknown reason %" PRId64 "\n", result);

    if (!aeron_publication_is_connected(self->publication))
        printf("No active subscribers detected\n");

    return PyLong_FromLong(result);
}

static PyMethodDef Publisher_methods[] = {
    { "offer", (PyCFunction)Publisher_offer, METH_VARARGS,
        "Return the new stream position otherwise a negative error value."
    },
    { NULL }
};

static PyTypeObject PublisherType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "aeron.Publisher",
    .tp_doc = "The client application which produces messages by offering them into a Publication",
    .tp_basicsize = sizeof(PublisherObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Publisher_new,
    .tp_init = (initproc)Publisher_init,
    .tp_dealloc = (destructor)Publisher_dealloc,
    .tp_methods = Publisher_methods,
};

static PyModuleDef aeronmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "aeron",
    .m_doc = "Efficient reliable UDP unicast, UDP multicast, and IPC message transport",
    .m_size = -1,
};

__attribute__((unused)) PyMODINIT_FUNC PyInit_aeron(void)
{
    PyObject* module;
    if (PyType_Ready(&SubscriberType) < 0)
        return NULL;
    if (PyType_Ready(&PublisherType) < 0)
        return NULL;

    module = PyModule_Create(&aeronmodule);
    if (module == NULL)
        return NULL;

    Py_INCREF(&SubscriberType);
    if (PyModule_AddObject(module, "Subscriber", (PyObject*)&SubscriberType) < 0)
    {
        Py_DECREF(&SubscriberType);
        Py_DECREF(module);
        return NULL;
    }

    Py_INCREF(&PublisherType);
    if (PyModule_AddObject(module, "Publisher", (PyObject*)&PublisherType) < 0)
    {
        Py_DECREF(&PublisherType);
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
