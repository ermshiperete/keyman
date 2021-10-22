# vim:set noet ts=4:
#
# ibus-keyman unit tests
#
# Copyright (c) 2021 SIL International
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

AM_CFLAGS = \
  @IBUS_CFLAGS@ \
  @GTK_CFLAGS@ \
  @GIO_CFLAGS@ \
  @JSON_GLIB_CFLAGS@ \
  @X11_CFLAGS@ \
  @KEYMAN_PROC_CFLAGS@ \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  $(NULL)

AM_LDFLAGS = \
  @IBUS_LIBS@ \
  @GTK_LIBS@ \
  @GIO_LIBS@ \
  @X11_LIBS@ \
  @JSON_GLIB_LIBS@ \
  @KEYMAN_PROC_LIBS@ \
  $(NULL)

libexec_PROGRAMS = ibus-keyman-tests

ibus_keyman_tests_SOURCES = \
  ibusimcontext.c \
  ibusimcontext.h \
  testenvironment.cpp \
  testenvironment.hpp \
  testmodule.c \
  testmodule.h \
  testfixture.cpp \
  $(NULL)

ibus_keyman_tests_CFLAGS = \
  @IBUS_CFLAGS@ \
  @GTK_CFLAGS@ \
  @GIO_CFLAGS@ \
  @X11_CFLAGS@ \
  @JSON_GLIB_CFLAGS@ \
  @KEYMAN_PROC_CFLAGS@ \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  $(NULL)

ibus_keyman_tests_CPPFLAGS = \
  @IBUS_CFLAGS@ \
  @GTK_CFLAGS@ \
  @GIO_CFLAGS@ \
  @X11_CFLAGS@ \
  @JSON_GLIB_CFLAGS@ \
  @KEYMAN_PROC_CFLAGS@ \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  $(NULL)

ibus_keyman_tests_LDFLAGS = \
  @GTK_LIBS@ \
  @GIO_LIBS@ \
  @IBUS_LIBS@ \
  @X11_LIBS@ \
  @JSON_GLIB_LIBS@ \
  @KEYMAN_PROC_LIBS@ \
  $(NULL)
