NULL =

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	$(GLIB_CFLAGS)				\
	$(GIO_CFLAGS)				\
	-DLIBEXECDIR=\"$(libexecdir)\"		\
	-DBINDIR=\"$(bindir)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	$(NULL)

bin_PROGRAMS =					\
	pipe-reader				\
	pipe-writer				\
	dls					\
	dpwd					\
	dresolve				\
	decho					\
	dcat					\
	darg					\
	dcall					\
	$(NULL)

pipe_reader_SOURCES = pipe-reader.c pipeneg.c
pipe_writer_SOURCES = pipe-writer.c pipeneg.c
dls_SOURCES = dls.c dtools.c
dls_LDADD = $(GIO_LIBS)
dpwd_SOURCES = dpwd.c dtools.c
dpwd_LDADD = $(GIO_LIBS)
dresolve_SOURCES = dresolve.c dtools.c
dresolve_LDADD = $(GIO_LIBS)
decho_SOURCES = decho.c variant-writer.c pipeneg.c
decho_LDADD = $(GIO_LIBS)
dcall_SOURCES = dcall.c variant-writer.c pipeneg.c dtools.c
dcall_LDADD = $(GIO_LIBS)
dcat_SOURCES = dcat.c variant-reader.c pipeneg.c
dcat_LDADD = $(GIO_LIBS)
darg_SOURCES = darg.c variant-reader.c pipeneg.c
darg_LDADD = $(GIO_LIBS)

ChangeLog:
	$(AM_V_GEN) if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git ./missing --run git log --stat) | fmt --split-only > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi


EXTRA_DIST=
