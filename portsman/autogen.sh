#!/bin/sh
# run this script to generate a configure script
aclocal \
&& automake --add-missing -i\
&& autoconf
