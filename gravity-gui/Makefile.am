
#DEBUG_OR_NOT= -ggdb3
DEBUG_OR_NOT= -O2
AUTOMAKE_OPTIONS= -Wno-portability

AM_CXXFLAGS = $(DEBUG_OR_NOT) -Wall
AM_CPPFLAGS = $(DEBUG_OR_NOT) -DDATADIR=\"$(pkgdatadir)/$(curr_data)\" -DVERSION=\"$(VERSION)\"
AM_CFLAGS =$(DEBUG_OR_NOT) -Wall -std=c99 -pg

LDADD = -lm -lncurses
AM_LDFLAGS =

bin_PROGRAMS = gravity_gui
noinst_PROGRAMS =
dist_bin_SCRIPTS = 
dist_pkgdata_DATA = gravity-gui.png
doc_DATA = 

dist_doc_DATA=COPYING LICENSE README COPYRIGHTS ChangeLog \
              Gravity_Manual_17-Oct-2017_rev_1.3.doc \
              Gravity_Manual_17-Oct-2017_rev_1.3.pdf


BUILT_SOURCES = ui_gravity_gui.h ui_helpbox.h qrc_gravity_gui.cpp moc_gravity_gui.cpp moc_ReplWidget.cpp moc_g_prog.cpp moc_helpbox.cpp Makefile.qt

gravity_code = main.cpp \
                 gravity_gui.cpp \
					  g_gui_impl.cpp \
					  g_progs_impl.cpp \
					  g_prog.cpp \
					  g_prog.h \
					  gravity_gui.ui \
					  gravity_gui.h \
					  gravity_gui.qrc \
					  gravity_gui.ui \
					  gravity_gui.pro \
					  gravity-gui.png \
					  ReplWidget.cpp \
					  ReplWidget.h \
					  helpbox.cpp \
					  helpbox.h \
					  helpbox.ui

gravity_gui_SOURCES = $(gravity_code) $(BUILT_SOURCES)

CLEANFILES = ${BUILT_SOURCES}

EXTRA_DIST = debian gravity-gui.png

gravity_gui_CXXFLAGS = `pkg-config --cflags Qt5Gui Qt5Core Qt5Widgets` -m64 -pipe -O2 -Wall -W -D_REENTRANT -fPIC ${DEFINES}
gravity_gui_LDFLAGS = `pkg-config --libs Qt5Gui Qt5Core Qt5Widgets`

Makefile.qt: Makefile
	qmake $(srcdir)/gravity_gui.pro -r 'DEFINES+=VERSION=\\\"$(VERSION)\\\"' 'DEFINES+=DEBUG_OR_NOT=\\\"$(DEBUG_OR_NOT)\\\"'

checkin_files=$(gravity_code) $(EXTRA_DIST) Makefile.am configure.ac $(dist_doc_DATA)

checkin_release:
	git add $(checkin_files) && git commit -uno -S -q -m "Release files for version $(VERSION)"

checkpoint:
	git add --verbose $(checkin_files) && git commit -uno -S -q -m "checkpoint files for version $(VERSION)"

checkpoint_withcomment:
	git add --verbose $(checkin_files) && git commit -uno -S -q
moc_%.cpp: %.h
	@MOC@ -o $@ $<

ui_%.h: %.ui
	@UIC@ -o $@ $<

qrc_%.cpp: %.qrc
	@RCC@ -o $@ $<

deb:
	@echo 'Making debian packages'
	make distdir &&\
	cd $(distdir) &&\
	dpkg-buildpackage -Zgzip -uc -us && \
	cd .. && \
	rm -rf $(distdir)

