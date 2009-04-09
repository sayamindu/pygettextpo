#include <Python.h>
#include <gettext-po.h>

typedef struct {
	PyObject_HEAD po_file_t pofile;
} PoFile;

typedef struct {
	PyObject_HEAD po_message_t pomessage;
} PoMessage;

typedef struct {
	PyObject_HEAD po_message_iterator_t pm_it;
} PoMessageIterator;


static PyObject *PoMessageIterator_iter(PoFile * pofile);
static void PoMessageIterator_dealloc(PoMessageIterator * it);
static PyObject *PoMessageIterator_next(PoMessageIterator * it);


/****************************************
                PoFile
****************************************/

static int
PoFile_init(PoFile * self, PyObject *args, PyObject *kwds)
{
	self->pofile = po_file_create();
	return 0;
}

static void
PoFile_dealloc(PoFile * self)
{
	po_file_free(self->pofile);
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject *
PoFile_read(PoFile * self, PyObject *args)
{
	const char *filename;
	po_xerror_handler_t error_handle;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	error_handle =
		(po_xerror_handler_t) malloc(sizeof(po_xerror_handler_t));

	/* We need to free the existing pofile first */
	if (self->pofile != NULL)
		po_file_free(self->pofile);

	self->pofile = po_file_read(filename, error_handle);

	free((void *)error_handle);

	if (self->pofile == NULL)
		return PyBool_FromLong(0);	/* TODO: Raise exception */
	else
		return PyBool_FromLong(1);
}

static PyObject *
PoFile_write(PoFile * self, PyObject *args)
{
	const char *filename;
	po_xerror_handler_t error_handle;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	error_handle =
		(po_xerror_handler_t) malloc(sizeof(po_xerror_handler_t));

	self->pofile = po_file_write(self->pofile, filename, error_handle);

	free((void *)error_handle);

	if (self->pofile == NULL)
		return PyBool_FromLong(0);	/* TODO: Raise exception */
	else
		return PyBool_FromLong(1);
}

static PyObject *
PoFile_domains(PoFile * self, PyObject *args)
{
	PyObject *domain_list;
	const char *const *domains;
	const char *const *domainp;

	domain_list = PyList_New(0);
	domains = po_file_domains(self->pofile);

	for (domainp = domains; *domainp; domainp++)
		PyList_Append(domain_list, Py_BuildValue("s", *domainp));

	return domain_list;
}

static PyMethodDef PoFile_methods[] = {
	{"read", (PyCFunction) PoFile_read, METH_VARARGS,
	 "Reads a PO file"},
	{"write", (PyCFunction) PoFile_write, METH_VARARGS,
	 "Writes a PO file"},
	{"domains", (PyCFunction) PoFile_domains, METH_VARARGS,
	 "Gets list of domains in PO file"},
	{NULL}
};

static PyTypeObject PoFileType = {
	PyObject_HEAD_INIT(NULL)
		0,		/* ob_size */
	"PoFile",		/* tp_name */
	sizeof(PoFile),		/* tp_basicsize */
	0,			/* tp_itemsize */
	(destructor) PoFile_dealloc,	/* tp_dealloc */
	0,			/* tp_print */
	0,			/* tp_getattr */
	0,			/* tp_setattr */
	0,			/* tp_compare */
	0,			/* tp_repr */
	0,			/* tp_as_number */
	0,			/* tp_as_sequence */
	0,			/* tp_as_mapping */
	0,			/* tp_hash */
	0,			/* tp_call */
	0,			/* tp_str */
	0,			/* tp_getattro */
	0,			/* tp_setattro */
	0,			/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_LIST_SUBCLASS,	/* tp_flags */
	"PoFile object",	/* tp_doc */
	0,			/* tp_traverse */
	0,			/* tp_clear */
	0,			/* tp_richcompare */
	0,			/* tp_weaklistoffset */
	(getiterfunc) PoMessageIterator_iter,	/* tp_iter */
	0,			/* tp_iternext */
	PoFile_methods,		/* tp_methods */
	0,			/* tp_members */
	0,			/* tp_getset */
	0,			/* tp_base */
	0,			/* tp_dict */
	0,			/* tp_descr_get */
	0,			/* tp_descr_set */
	0,			/* tp_dictoffset */
	(initproc) PoFile_init,	/* tp_init */
	0,			/* tp_alloc */
	0,			/* tp_new */
};

/****************************************
                PoMessage
****************************************/

static int
PoMessage_init(PoMessage * self, PyObject *args, PyObject *kwds)
{
	self->pomessage = NULL;
	return 0;
}

static void
PoMessage_dealloc(PoMessage * self)
{
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject *
PoMessage_msgctxt(PoMessage * self, PyObject *args)
{
	return Py_BuildValue("s", po_message_msgctxt(self->pomessage));
}

static PyObject *
PoMessage_set_msgctxt(PoMessage * self, PyObject *args)
{
	char *msgctxt;

	if (!PyArg_ParseTuple(args, "s", &msgctxt))
		msgctxt = NULL;
	po_message_set_msgctxt(self->pomessage, msgctxt);
	Py_RETURN_NONE;
}

static PyObject *
PoMessage_msgid(PoMessage * self, PyObject *args)
{
	return Py_BuildValue("s", po_message_msgid(self->pomessage));
}

static PyObject *
PoMessage_set_msgid(PoMessage * self, PyObject *args)
{
	char *msgid;

	if (!PyArg_ParseTuple(args, "s", &msgid))
		return NULL;
	po_message_set_msgid(self->pomessage, msgid);
	Py_RETURN_NONE;
}

static PyObject *
PoMessage_msgid_plural(PoMessage * self, PyObject *args)
{
	return Py_BuildValue("s", po_message_msgid_plural(self->pomessage));
}

static PyObject *
PoMessage_set_msgid_plural(PoMessage * self, PyObject *args)
{
	char *msgid_plural;

	if (!PyArg_ParseTuple(args, "s", &msgid_plural))
		return NULL;
	po_message_set_msgid_plural(self->pomessage, msgid_plural);
	Py_RETURN_NONE;
}

static PyObject *
PoMessage_msgstr(PoMessage * self, PyObject *args)
{
	return Py_BuildValue("s", po_message_msgstr(self->pomessage));
}

static PyObject *
PoMessage_set_msgstr(PoMessage * self, PyObject *args)
{
	char *msgstr;

	if (!PyArg_ParseTuple(args, "s", &msgstr))
		return NULL;
	po_message_set_msgstr(self->pomessage, msgstr);
	Py_RETURN_NONE;
}

static PyObject *
PoMessage_msgstr_plural(PoMessage * self, PyObject *args)
{
	int index;
	const char *retvalue;

	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	retvalue = po_message_msgstr_plural(self->pomessage, index);

	if (retvalue == NULL)
		Py_RETURN_NONE;
	else
		return Py_BuildValue("s", retvalue);
}

static PyObject *
PoMessage_set_msgstr_plural(PoMessage * self, PyObject *args)
{
	int index;
	char *msgstr = NULL;

	if (!PyArg_ParseTuple(args, "i|s", &index, &msgstr))
		return NULL;

	po_message_set_msgstr_plural(self->pomessage, index, msgstr);
	Py_RETURN_NONE;
}

static PyObject *
PoMessage_comments(PoMessage * self, PyObject *args)
{
	return Py_BuildValue("s", po_message_comments(self->pomessage));
}

static PyObject *
PoMessage_set_comments(PoMessage * self, PyObject *args)
{
	char *comments;

	if (!PyArg_ParseTuple(args, "s", &comments))
		return NULL;
	po_message_set_comments(self->pomessage, comments);
	Py_RETURN_NONE;
}


static PyMethodDef PoMessage_methods[] = {
	{"msgctxt", (PyCFunction) PoMessage_msgctxt, METH_NOARGS,
	 "Returns the msgctxt of the message"},
	{"set_msgctxt", (PyCFunction) PoMessage_set_msgctxt, METH_VARARGS,
	 "Sets the msgctxt of the message"},
	{"msgid", (PyCFunction) PoMessage_msgid, METH_NOARGS,
	 "Returns the msgid of the message"},
	{"set_msgid", (PyCFunction) PoMessage_set_msgid, METH_VARARGS,
	 "Sets the msgid of the message"},
	{"msgid_plural", (PyCFunction) PoMessage_msgid_plural, METH_NOARGS,
	 "Returns the msgid_plural (untranslated English plural string) of the message"},
	{"set_msgid_plural", (PyCFunction) PoMessage_set_msgid_plural,
	 METH_VARARGS,
	 "Sets the plural msgid of the message"},
	{"msgstr", (PyCFunction) PoMessage_msgstr, METH_NOARGS,
	 "Returns the msgstr of the message"},
	{"set_msgstr", (PyCFunction) PoMessage_set_msgstr, METH_VARARGS,
	 "Sets the msgstr of the message"},
	{"msgstr_plural", (PyCFunction) PoMessage_msgstr_plural, METH_NOARGS,
	 "Returns the msgstr[index] for a message with plural handling of the message"},
	{"set_msgstr_plural", (PyCFunction) PoMessage_set_msgstr_plural,
	 METH_VARARGS,
	 "Sets the msgstr[index] of the message with plural form"},
	{"comments", (PyCFunction) PoMessage_comments, METH_NOARGS,
	 "Returns the comments of the message"},
	{"set_comments", (PyCFunction) PoMessage_set_comments, METH_VARARGS,
	 "Sets the comments of the message"},
	{NULL}
};

static PyTypeObject PoMessageType = {
	PyObject_HEAD_INIT(NULL)
		0,		/* ob_size */
	"PoMessage",		/* tp_name */
	sizeof(PoMessage),	/* tp_basicsize */
	0,			/* tp_itemsize */
	(destructor) PoMessage_dealloc,	/* tp_dealloc */
	0,			/* tp_print */
	0,			/* tp_getattr */
	0,			/* tp_setattr */
	0,			/* tp_compare */
	0,			/* tp_repr */
	0,			/* tp_as_number */
	0,			/* tp_as_sequence */
	0,			/* tp_as_mapping */
	0,			/* tp_hash */
	0,			/* tp_call */
	0,			/* tp_str */
	0,			/* tp_getattro */
	0,			/* tp_setattro */
	0,			/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	"PoMessage object",	/* tp_doc */
	0,			/* tp_traverse */
	0,			/* tp_clear */
	0,			/* tp_richcompare */
	0,			/* tp_weaklistoffset */
	0,			/* tp_iter */
	0,			/* tp_iternext */
	PoMessage_methods,	/* tp_methods */
	0,			/* tp_members */
	0,			/* tp_getset */
	0,			/* tp_base */
	0,			/* tp_dict */
	0,			/* tp_descr_get */
	0,			/* tp_descr_set */
	0,			/* tp_dictoffset */
	(initproc) PoMessage_init,	/* tp_init */
	0,			/* tp_alloc */
	0,			/* tp_new */
};


/****************************************
            PoMessageIterator
****************************************/

PyTypeObject PoMessageIteratorType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
		"PoMessageIterator",	/* tp_name */
	sizeof(PoMessageIterator),	/* tp_basicsize */
	0,			/* tp_itemsize */
	/* methods */
	(destructor) PoMessageIterator_dealloc,	/* tp_dealloc */
	0,			/* tp_print */
	0,			/* tp_getattr */
	0,			/* tp_setattr */
	0,			/* tp_compare */
	0,			/* tp_repr */
	0,			/* tp_as_number */
	0,			/* tp_as_sequence */
	0,			/* tp_as_mapping */
	0,			/* tp_hash */
	0,			/* tp_call */
	0,			/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	0,			/* tp_setattro */
	0,			/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
	0,			/* tp_doc */
	0,			/* tp_traverse */
	0,			/* tp_clear */
	0,			/* tp_richcompare */
	0,			/* tp_weaklistoffset */
	PyObject_SelfIter,	/* tp_iter */
	(iternextfunc) PoMessageIterator_next,	/* tp_iternext */
	0,			/* tp_methods */
	0,			/* tp_members */
};

static PyObject *
PoMessageIterator_iter(PoFile * pofile)
{
	PoMessageIterator *it;

	it = PyObject_GC_New(PoMessageIterator, &PoMessageIteratorType);
	if (it == NULL)
		return NULL;
	it->pm_it = po_message_iterator(pofile->pofile, NULL);	/* How do I specify another domain ? */
	PyObject_GC_Track(it);
	return (PyObject *)it;
}

static void
PoMessageIterator_dealloc(PoMessageIterator * self)
{
	PyObject_GC_UnTrack(self);
	po_message_iterator_free(self->pm_it);
	PyObject_GC_Del(self);
}

static PyObject *
PoMessageIterator_next(PoMessageIterator * it)
{
	PoMessage *message;

	message = PyObject_New(PoMessage, &PoMessageType);
	message->pomessage = po_next_message(it->pm_it);

	if (message->pomessage == NULL) {
		return NULL;
	}
	else
		return (PyObject *)message;
}

/******************** Module Initialization function ****************/

void
initgettextpo(void)
{
	PyObject *mod;

	// Create the module
	mod = Py_InitModule3("gettextpo", NULL,
			     "An extension for manipulating PO files");
	if (mod == NULL) {
		return;
	}

	// Fill in some slots in the type, and make it ready
	PoFileType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&PoFileType) < 0) {
		return;
	}
	// Add the type to the module.
	Py_INCREF(&PoFileType);
	PyModule_AddObject(mod, "PoFile", (PyObject *)&PoFileType);

	// Fill in some slots in the type, and make it ready
	PoMessageIteratorType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&PoMessageIteratorType) < 0) {
		return;
	}
	// Add the type to the module.
	Py_INCREF(&PoMessageIteratorType);
	PyModule_AddObject(mod, "PoMessageIterator",
			   (PyObject *)&PoMessageIteratorType);

	// Fill in some slots in the type, and make it ready
	PoMessageType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&PoMessageType) < 0) {
		return;
	}
	// Add the type to the module.
	Py_INCREF(&PoMessageType);
	PyModule_AddObject(mod, "PoMessage", (PyObject *)&PoMessageType);
}
