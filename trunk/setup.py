#!/usr/bin/python
from distutils.core import setup, Extension

main = Extension(	'gettextpo', 
			define_macros = [('_LINUX',None)],
			libraries = ['gettextpo'],
			include_dirs = [],
			sources = ['gettextpo.c'],
			extra_compile_args = ['-Wall'])

setup(	name = "gettextpo",
	version = "0.1",
	description = "Module for manipulating PO files",
    author="Sayamindu Dasgupta",
    author_email="sayamindu@gmail.com",
    url="http://code.google.com/p/pygettextpo",
	ext_modules = [main])
