#
# Makefile for lib/et
#

srcdir = .
top_srcdir = ../..

top_builddir = ../..
my_dir = lib/et
INSTALL = /data/data/com.termux/files/usr/bin/install -c
MKDIR_P = /data/data/com.termux/files/usr/bin/mkdir -p

DEP_MAKEFILE = $(DEP_LIB_MAKEFILES)

# Beginning of file MCONFIG

all::

all-static::

check::

fullcheck::

SHELL = /bin/sh

COMPRESS_EXT = gz bz2 bz Z

prefix = /usr
root_prefix = 
exec_prefix = ${prefix}
root_bindir = ${root_prefix}/bin
root_sbindir = ${root_prefix}/sbin
root_libdir = ${root_prefix}/lib
datarootdir = ${prefix}/share
bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libdir = ${exec_prefix}/lib
datadir= ${datarootdir}
localedir = $(datadir)/locale
root_sysconfdir= ${root_prefix}/etc
includedir = ${prefix}/include
mandir = ${datarootdir}/man
man1dir = $(mandir)/man1
man3dir = $(mandir)/man3
man5dir = $(mandir)/man5
man8dir = $(mandir)/man8
infodir = ${datarootdir}/info
datadir = ${datarootdir}
pkgconfigdir = $(libdir)/pkgconfig
pkglibdir = $(libdir)/e2fsprogs

HAVE_UDEV = no
UDEV_RULES_DIR = 
HAVE_CROND = no
CROND_DIR = 
HAVE_SYSTEMD = no
SYSTEMD_SYSTEM_UNIT_DIR = 



 V =
 ifeq ($(strip $(V)),)
 #  E = @echo
 #  ES = echo
 #  Q = @
    E = @echo
    ES = echo
    Q = @
 else
    E = @\#
    ES = \#
    Q =
 endif

# E = @echo
# ES = echo
# Q = @

 CHECK=sparse
 CHECK_OPTS=-Wsparse-all -Wno-transparent-union -Wno-return-void -Wno-undef -Wno-non-pointer-null
 CPPCHECK=cppcheck
 CPPCHECK_OPTS=--force --enable=all --quiet
 ifeq ("$(C)", "2")
   CHECK_CMD=$(CHECK) $(CHECK_OPTS) -Wbitwise -D__CHECK_ENDIAN__
   CPPCHECK_CMD=$(CPPCHECK) $(CPPCHECK_OPTS)
 else
   ifeq ("$(C)", "1")
     CHECK_CMD=$(CHECK) $(CHECK_OPTS)
     CPPCHECK_CMD=$(CPPCHECK) $(CPPCHECK_OPTS)
    else
     CHECK_CMD=@true
     CPPCHECK_CMD=@true
   endif
 endif

# CHECK_CMD=true
# CPPCHECK_CMD=true

SANITIZER_CFLAGS =    
SANITIZER_LDFLAGS =    

CC = gcc
CXX = 
BUILD_CC = gcc
PTHREAD_CFLAGS = -pthread
PTHREAD_LIBS = -lpthread
CFLAGS = -g -O2
CFLAGS_SHLIB = -g -O2
CFLAGS_STLIB = -g -O2
CPPFLAGS = -I. -I$(top_builddir)/lib -I$(top_srcdir)/lib
ALL_CFLAGS = $(CPPFLAGS) $(SANITIZER_CFLAGS) $(CFLAGS) $(PTHREAD_CFLAGS) $(CFLAGS_WARN) -DHAVE_CONFIG_H $(LOCAL_CFLAGS)
ALL_CFLAGS_SHLIB = $(CPPFLAGS) $(SANITIZER_CFLAGS) $(CFLAGS_SHLIB) $(PTHREAD_CFLAGS) $(CFLAGS_WARN) -DHAVE_CONFIG_H $(LOCAL_CFLAGS)
ALL_CFLAGS_STLIB = $(CPPFLAGS) $(SANITIZER_CFLAGS) $(CFLAGS_STLIB) $(PTHREAD_CFLAGS) $(CFLAGS_WARN) -DHAVE_CONFIG_H $(LOCAL_CFLAGS)
LDFLAGS = $(SANITIZER_LDFLAGS) $(PTHREAD_CFLAGS) 
LDFLAGS_SHLIB = $(SANITIZER_LDFLAGS) $(PTHREAD_CFLAGS) 
ALL_LDFLAGS = $(LDFLAGS)  $(LOCAL_LDFLAGS)
LDFLAGS_STATIC = $(SANITIZER_LDFLAGS) $(PTHREAD_CFLAGS) 
BUILD_CFLAGS = $(SANITIZER_CFLAGS) -g -O2  -I. -I$(top_builddir)/lib -I$(top_srcdir)/lib -DHAVE_CONFIG_H
BUILD_LDFLAGS = $(SANITIZER_LDFLAGS) 
RDYNAMIC = -rdynamic
LINK_BUILD_FLAGS = 
LINK_INSTALL_FLAGS = -f
RM = /data/data/com.termux/files/usr/bin/rm
LN = /data/data/com.termux/files/usr/bin/ln
LN_S = ln -s
MV = /data/data/com.termux/files/usr/bin/mv
CP = /data/data/com.termux/files/usr/bin/cp
CHMOD = /data/data/com.termux/files/usr/bin/chmod
AR = ar
AWK = gawk
SED = /data/data/com.termux/files/usr/bin/sed
PERL = perl
RANLIB = :
STRIP = :
LD = $(PURE) gcc
ARUPD = $(AR) r
ARGEN = $(AR) rc
LDCONFIG = :
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}

#
# Library definitions
#
LIB = $(top_builddir)/lib
LIBSS = $(LIB)/libss.a  -ldl
LIBCOM_ERR = $(LIB)/libcom_err.a  
LIBE2P = $(LIB)/libe2p.a
LIBEXT2FS = $(LIB)/libext2fs.a
LIBUUID = -L/data/data/com.termux/files/usr/lib -luuid 
LIBMAGIC = -ldl
LIBFUSE = 
LIBSUPPORT = $(LIBINTL) $(LIB)/libsupport.a
LIBBLKID = $(LIB)/libblkid.a  $(LIBUUID)
LIBINTL = 
SYSLIBS =  -lpthread
DEPLIBSS = $(LIB)/libss.a
DEPLIBCOM_ERR = $(LIB)/libcom_err.a
DEPLIBUUID = 
DEPLIBSUPPORT = $(LIB)/libsupport.a
DEPLIBBLKID = $(LIB)/libblkid.a  $(DEPLIBUUID)
TESTENV = LD_LIBRARY_PATH="$(LIB):$${LD_LIBRARY_PATH}" DYLD_LIBRARY_PATH="$(LIB):$${DYLD_LIBRARY_PATH}"

STATIC_LIBSS = $(LIB)/libss.a -ldl
STATIC_LIBCOM_ERR = $(LIB)/libcom_err.a 
STATIC_LIBE2P = $(LIB)/libe2p.a
STATIC_LIBEXT2FS = $(LIB)/libext2fs.a
STATIC_LIBUUID = -L/data/data/com.termux/files/usr/lib -luuid 
STATIC_LIBSUPPORT = $(LIBINTL) $(LIBSUPPORT)
STATIC_LIBBLKID = $(LIB)/libblkid.a $(STATIC_LIBUUID)
DEPSTATIC_LIBSS = $(LIB)/libss.a
DEPSTATIC_LIBCOM_ERR = $(LIB)/libcom_err.a
DEPSTATIC_LIBUUID = 
DEPSTATIC_LIBSUPPORT = $(DEPLIBSUPPORT)
DEPSTATIC_LIBBLKID = $(LIB)/libblkid.a $(DEPSTATIC_LIBUUID)

PROFILED_LIBSS = $(LIB)/libss.a -ldl
PROFILED_LIBCOM_ERR = $(LIB)/libcom_err.a 
PROFILED_LIBE2P = $(LIB)/libe2p.a
PROFILED_LIBEXT2FS = $(LIB)/libext2fs.a
PROFILED_LIBUUID = -L/data/data/com.termux/files/usr/lib -luuid 
PROFILED_LIBSUPPORT = $(LIBINTL) $(LIB)/libsupport.a
PROFILED_LIBBLKID = $(LIB)/libblkid.a $(PROFILED_LIBUUID)
DEPPROFILED_LIBSS = $(LIB)/libss.a
DEPPROFILED_LIBCOM_ERR = $(LIB)/libcom_err.a
DEPPROFILED_LIBUUID = -L/data/data/com.termux/files/usr/lib -luuid
DEPPROFILED_LIBSUPPORT = $(PROFILED_LIBSUPPORT)
DEPPROFILED_LIBBLKID = $(LIB)/libblkid.a $(DEPPROFILED_LIBUUID)

#
# A fast substitution command for fixing up man pages, shell scripts, etc.
#
SUBST_CONF=$(top_builddir)/util/subst.conf
SUBSTITUTE= $(top_builddir)/util/subst -f $(SUBST_CONF)
SUBSTITUTE_UPTIME= $(top_builddir)/util/subst -t -f $(SUBST_CONF)
DEP_SUBSTITUTE= $(top_builddir)/util/subst $(SUBST_CONF)

$(top_builddir)/util/subst:
	cd $(top_builddir)/util ; $(MAKE) subst

#
# Script for generating utf8data.h
#
MKUTF8DATA=$(top_builddir)/util/mkutf8data

$(top_builddir)/util/mkutf8data:
	 $(MAKE) -C $(top_builddir)/util mkutf8data

#
# Script for installing symlinks (for shared libraries)
#
$(top_builddir)/util/install-symlink: $(top_srcdir)/util/install-symlink.in \
			      $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=util/install-symlink ./config.status
	chmod +x $(top_builddir)/util/install-symlink

$(top_builddir)/util/symlinks:
	cd $(top_builddir)/util ; $(MAKE) symlinks

INSTALL_SYMLINK = /bin/sh $(top_builddir)/util/install-symlink \
			 \
			--symlinks=$(top_builddir)/util/symlinks
DEP_INSTALL_SYMLINK = $(top_builddir)/util/install-symlink \
			$(top_builddir)/util/symlinks

#
# Warning flags
#
# Run make gcc-wall to do a build with warning messages.
#
#
WFLAGS=		-std=gnu99 -D_XOPEN_SOURCE=600 -D_GNU_SOURCE $(WFLAGS_EXTRA) \
			-Wall -W -Wwrite-strings -Wpointer-arith \
			-Wcast-qual -Wcast-align -Wno-variadic-macros \
			-Wstrict-prototypes -Wmissing-prototypes \
			-Wformat-security  -Wformat-nonliteral \
			-Wmissing-format-attribute -O2 -Wstrict-aliasing \
			-Wnested-externs -Winline -DNO_INLINE_FUNCS -Wshadow \
			-UENABLE_NLS

gcc-wall-new:
	($(MAKE) CFLAGS_WARN="$(WFLAGS)" > /dev/null) 2>&1

gcc-wall:
	$(MAKE) clean > /dev/null
	$(MAKE) gcc-wall-new

static-check:
	($(MAKE) C=1 V=1 CFLAGS="$(ALL_CFLAGS) $(WFLAGS)") 2>&1

static-check-all:
	$(MAKE) clean > /dev/null
	$(MAKE) static-check

#
# Installation user and groups
#
BINGRP=		bin
BINOWN=		bin
BINMODE=	555
INCGRP=		bin
INCOWN=		bin
INCMODE=	444
LIBOWN=		bin
LIBGRP=		bin
LIBMODE=	444
MANGRP=		bin
MANOWN=		bin
MANMODE=	444

#
# Autoconf magic...
#

DEP_LIB_MAKEFILES = $(top_srcdir)/lib/Makefile.library \
	$(top_srcdir)/lib/Makefile.elf-lib  \
	$(top_srcdir)/lib/Makefile.bsd-lib \
	$(top_srcdir)/lib/Makefile.darwin-lib \
	$(top_srcdir)/lib/Makefile.solaris-lib \
	$(top_srcdir)/lib/Makefile.profile

$(top_builddir)/config.status: $(top_srcdir)/configure
	cd $(top_builddir); ./config.status --recheck

$(top_builddir)/MCONFIG: $(top_srcdir)/MCONFIG.in $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=MCONFIG ./config.status

$(top_builddir)/lib/config.h: $(top_srcdir)/lib/config.h.in \
			      $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=lib/config.h ./config.status

$(top_builddir)/lib/dirpaths.h: $(DEP_SUBSTITUTE) $(top_srcdir)/lib/dirpaths.h.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE) $(top_srcdir)/lib/dirpaths.h.in $@

$(top_builddir)/lib/substitute_sh: $(top_srcdir)/lib/substitute_sh.in \
		$(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=lib/substitute_sh ./config.status

$(top_builddir)/util/subst.conf: $(top_srcdir)/util/subst.conf.in \
		$(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=util/subst.conf ./config.status

Makefile: $(srcdir)/Makefile.in $(top_builddir)/MCONFIG \
		$(DEP_MAKEFILE) $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=$(my_dir)/Makefile ./config.status

#$(top_srcdir)/configure: $(top_srcdir)/configure.ac
#	cd $(top_srcdir) && autoheader && autoconf

coverage.txt: Makefile $(SRCS)
	if test -n "$(SRCS)"; then \
		gcov -s $(top_srcdir) -o . $(SRCS) > coverage.txt 2>&1 ; \
	fi

clean::
	$(RM) -f *.gcda *.gcov *.gcno coverage.txt

#
# Make depend magic...
#

.depend: Makefile $(SRCS) $(top_srcdir)/depfix.sed $(top_srcdir)/wordwrap.pl
	if test -n "$(SRCS)" ; then \
		$(CC) -M $(ALL_CFLAGS) $(DEPEND_CFLAGS) $(SRCS) | \
			$(SED) -f $(top_srcdir)/depfix.sed \
			    -e 's; $(srcdir)/; $$(srcdir)/;g' \
			    -e 's; $(top_srcdir)/; $$(top_srcdir)/;g' \
			    -e 's; $(top_builddir)/; $$(top_builddir)/;g' \
			    -e 's; \./; ;g' \
			    -e '/^#/d' \
			    -e '/^ *\\$$/d' | \
			$(PERL) $(top_srcdir)/wordwrap.pl > .depend; \
	else :; fi

depend:: .depend
	if test -n "$(SRCS)" ; then \
		sed -e '/^# +++ Dependency line eater +++/,$$d' \
			< $(srcdir)/Makefile.in | cat - .depend \
			> $(srcdir)/Makefile.in.new; \
	if cmp -s $(srcdir)/Makefile.in $(srcdir)/Makefile.in.new ; then \
		$(RM) $(srcdir)/Makefile.in.new ; \
	else \
		$(MV) $(srcdir)/Makefile.in $(srcdir)/Makefile.in.old; \
		$(MV) $(srcdir)/Makefile.in.new $(srcdir)/Makefile.in; \
	fi ; else :; fi

# End of file MCONFIG

all:: compile_et com_err.pc

OBJS= error_message.o et_name.o init_et.o com_err.o com_right.o
SRCS = $(srcdir)/error_message.c $(srcdir)/et_name.c $(srcdir)/init_et.c \
	$(srcdir)/com_err.c $(srcdir)/com_right.c

HFILES= com_err.h
SHARE_FILES= et_c.awk et_h.awk

LIBRARY= libcom_err
LIBDIR= et

ELF_VERSION = 2.1
ELF_SO_VERSION = 2
ELF_IMAGE = libcom_err
ELF_MYDIR = et
ELF_INSTALL_DIR = $(root_libdir)
ELF_OTHER_LIBS = 

BSDLIB_VERSION = 1.1
BSDLIB_IMAGE = libcom_err
BSDLIB_MYDIR = et
BSDLIB_INSTALL_DIR = $(root_libdir)

#
# what to build...
#
.c.o:
	$(E) "	CC $<"
	$(Q) $(CC) $(ALL_CFLAGS_STLIB) -c $< -o $@
	$(Q) $(CHECK_CMD) $(ALL_CFLAGS) $<
	$(Q) $(CPPCHECK_CMD) $(CPPFLAGS) $<
#	$(Q) $(CC) $(ALL_CFLAGS_STLIB) -g -pg -o profiled/$*.o -c $<
#	$(Q) $(CC) $(ALL_CFLAGS_SHLIB) -fPIC -shared -o elfshared/$*.o -c $<
#	$(Q) $(CC) $(ALL_CFLAGS_SHLIB) $(BSDLIB_PIC_FLAG) -o pic/$*.o -c $<

all:: subdirs $(LIBRARY).a

install-shlibs-strip::

install-shlibs::

uninstall-shlibs::

real-subdirs::

subdirs:: Makefile
	$(Q) $(MAKE) -s real-subdirs
	$(Q) touch subdirs

clean::
	$(Q) $(RM) -f subdirs

$(LIBRARY).a: $(OBJS)
	$(E) "	GEN_LIB $@"
	$(Q) (if test -r $@; then $(RM) -f $@.bak && $(MV) $@ $@.bak; fi)
	$(Q) $(ARGEN) $@ $(OBJS)
	-@$(RANLIB) $@
	$(Q) $(RM) -f ../$@
	$(Q) (cd ..; $(LN) $(LINK_BUILD_FLAGS) \
		`echo $(my_dir) | sed -e 's;lib/;;'`/$@ $@)

$(LIB)/$(LIBRARY).a: $(LIBRARY).a

compile_et: $(DEP_SUBSTITUTE) $(srcdir)/compile_et.sh.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE) $(srcdir)/compile_et.sh.in compile_et
	$(Q) $(CHMOD) +x compile_et

DVI=texi2dvi
DVIPS=dvips -o "$@"
INFO=echo Makeinfo is missing. Info documentation will not be built.
HTML=makeinfo --html --no-split
PS2PDF=ps2pdf

com_err.ps : com_err.dvi
com_err.dvi: com_err.texinfo

com_err.info: $(srcdir)/com_err.texinfo
	$(E) "	MAKEINFO $@"
	-$(Q) $(INFO) $(srcdir)/com_err.texinfo

com_err.dvi: $(srcdir)/com_err.texinfo
	$(E) "	TEXI2DVI $@"
	-$(Q) $(DVI) $(srcdir)/com_err.texinfo

com_err.ps: com_err.dvi
	$(E) "	DVIPS $@"
	-$(Q) $(DVIPS) com_err.dvi

com_err.pdf: com_err.ps
	$(E) "	PS2PDF $@"
	-$(Q) $(PS2PDF) com_err.ps

com_err.html: $(srcdir)/com_err.texinfo
	$(E) "	MAKEINFO $@"
	-$(Q) $(HTML) $(srcdir)/com_err.texinfo

com_err.pc: $(srcdir)/com_err.pc.in $(top_builddir)/config.status
	$(E) "	CONFIG.STATUS $@"
	$(Q) cd $(top_builddir); CONFIG_FILES=lib/et/com_err.pc ./config.status

#libcom_err.o:	$(LIBOBJS)
#	$(LD) -r -s -o libcom_err.o $(LIBOBJS)
#	chmod -x libcom_err.o

TAGS:	$(SRCS)
	$(TAGS) $(SRCS)

installdirs::
	$(E) "	MKDIR_P $(libdir) $(includedir)/et $(datadir)/et $(bindir) $(man1dir) $(man3dir)"
	$(Q) $(MKDIR_P) $(DESTDIR)$(libdir) \
		$(DESTDIR)$(includedir)/et $(DESTDIR)$(datadir)/et \
		$(DESTDIR)$(bindir) $(DESTDIR)$(man1dir) \
		$(DESTDIR)$(man3dir) $(DESTDIR)$(pkgconfigdir)

install:: compile_et libcom_err.a $(HFILES) installdirs com_err.pc
	$(E) "	INSTALL_DATA $(libdir)/libcom_err.a"
	$(Q) $(INSTALL_DATA) libcom_err.a $(DESTDIR)$(libdir)/libcom_err.a
	-$(Q) $(RANLIB) $(DESTDIR)$(libdir)/libcom_err.a
	$(Q) $(CHMOD) $(LIBMODE) $(DESTDIR)$(libdir)/libcom_err.a
	$(Q) for i in $(HFILES); do \
		echo "	INSTALL_DATA $(includedir)/et/$$i"; \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(includedir)/et/$$i; \
	done
	$(Q) (cd $(DESTDIR)$(includedir) ;\
		$(LN) $(LINK_INSTALL_FLAGS) et/com_err.h . )
	$(Q) for i in $(SHARE_FILES); do \
		echo "	INSTALL_DATA $(datadir)/et/$$i"; \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(datadir)/et/$$i; \
	done
	$(E) "	INSTALL_SCRIPT $(bindir)/compile_et"
	$(Q) $(INSTALL_SCRIPT) compile_et $(DESTDIR)$(bindir)/compile_et
	$(E) "	INSTALL_DATA $(man3dir)/com_err.3"
	$(Q) $(INSTALL_DATA) $(srcdir)/com_err.3 $(DESTDIR)$(man3dir)/com_err.3
	$(E) "	INSTALL_DATA $(man1dir)/compile_et.1"
	$(Q) $(INSTALL_DATA) $(srcdir)/compile_et.1 \
			$(DESTDIR)$(man1dir)/compile_et.1
	$(E) "	INSTALL_DATA $(pkgconfigdir)/com_err.pc"
	$(Q) $(INSTALL_DATA) com_err.pc $(DESTDIR)$(pkgconfigdir)/com_err.pc

uninstall::
	$(RM) -f $(DESTDIR)$(libdir)/libcom_err.a \
		$(DESTDIR)$(bindir)/compile_et \
		$(DESTDIR)$(pkgconfigdir)/com_err.pc
	$(RM) -rf $(DESTDIR)$(includedir)/et $(DESTDIR)$(datadir)/et

fullcheck check:: compile_et
	for i in $(srcdir)/test_cases/*.et ; do \
		t=`basename $$i | sed -e 's/.et//'`; \
		_ET_DIR_OVERRIDE=$(srcdir) ./compile_et $$i ; \
		diff -c $(srcdir)/test_cases/$$t.c $$t.c > $$t.failed; \
		if [ $$? -ne 0 ]; then echo Test case $$t failed; exit 1 ; fi ; \
		diff -c $(srcdir)/test_cases/$$t.h $$t.h >> $$t.failed; \
		if [ $$? -ne 0 ]; then echo Test case $$t failed; exit 1 ; fi ; \
		$(RM) -f $$t.c $$t.h $$t.failed; \
		echo "Test case $$t succeeded" ; \
	done

clean::
	$(RM) -f compile_et libcom_err.a libcom_err_p.a com_err.info
	$(RM) -f $(OBJS) profiled/*
	$(RM) -f *~ \#* *.bak *.otl *.aux *.toc *.PS *.dvi *.ps TAGS *.ln \
		*.html *.cp *.fn *.fns *.ky *.log *.pc *.pg *.toc *.tp *.vr \
		*.pdf
	$(RM) -f ../libcom_err.a ../libcom_err_p.a

mostlyclean:: clean
distclean:: clean
	$(RM) -f .depend Makefile com_err.pc \
		$(srcdir)/TAGS $(srcdir)/Makefile.in.old

$(OBJS): subdirs

# +++ Dependency line eater +++
# 
# Makefile dependencies follow.  This must be the last section in
# the Makefile.in file
#
error_message.o: $(srcdir)/error_message.c $(top_builddir)/lib/config.h \
 $(top_builddir)/lib/dirpaths.h $(srcdir)/com_err.h $(srcdir)/error_table.h \
 $(srcdir)/internal.h
et_name.o: $(srcdir)/et_name.c $(top_builddir)/lib/config.h \
 $(top_builddir)/lib/dirpaths.h $(srcdir)/com_err.h $(srcdir)/error_table.h \
 $(srcdir)/internal.h
init_et.o: $(srcdir)/init_et.c $(top_builddir)/lib/config.h \
 $(top_builddir)/lib/dirpaths.h $(srcdir)/com_err.h $(srcdir)/error_table.h
com_err.o: $(srcdir)/com_err.c $(top_builddir)/lib/config.h \
 $(top_builddir)/lib/dirpaths.h $(srcdir)/com_err.h $(srcdir)/error_table.h \
 $(srcdir)/internal.h
com_right.o: $(srcdir)/com_right.c $(top_builddir)/lib/config.h \
 $(top_builddir)/lib/dirpaths.h $(srcdir)/com_err.h $(srcdir)/error_table.h
