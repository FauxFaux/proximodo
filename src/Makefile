CC = g++
# Replace the value below with the output of wx-config --cxxflags
WXINC = -I/usr/local/lib/wx/include/gtk2-ansi-release-2.5 -I/usr/local/include/wx-2.5 -DGTK_NO_CHECK_CASTS -D__WXGTK__ -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -DWX_PRECOMP -DNO_GCC_PRAGMA
# Replace the value below with the output of wx-config --libs
WXLIB = -L/usr/local/lib -pthread   -L/usr/X11R6/lib  -lwx_gtk2_xrc-2.5 -lwx_gtk2_html-2.5 -lwx_gtk2_adv-2.5 -lwx_gtk2_core-2.5 -lwx_base_xml-2.5 -lwx_base_net-2.5 -lwx_base-2.5
CFLAGS = -Wall -g -O2 $(WXINC)
SRC = configscreen.cpp controls.cpp descriptor.cpp editscreen.cpp events.cpp \
			expander.cpp filter.cpp filterowner.cpp giffilter.cpp headerfilter.cpp \
			log.cpp logframe.cpp main.cpp mainframe.cpp managerthread.cpp \
			matcher.cpp nodes.cpp platform.cpp proxy.cpp requestmanager.cpp \
			settings.cpp settingsscreen.cpp testframe.cpp textfilter.cpp \
			trayicon.cpp url.cpp util.cpp welcomescreen.cpp windowcontent.cpp \
			zlibbuffer.cpp
OBJ = $(SRC:.cpp=.o)
HDR = configscreen.h const.h controls.h descriptor.h editscreen.h events.h \
			expander.h filter.h filterowner.h giffilter.h headerfilter.h logframe.h \
			log.h logviewer.h mainframe.h managerthread.h matcher.h memory.h node.h \
			nodes.h platform.h Proximodo_private.h proxy.h receptor.h \
			requestmanager.h settings.h settingsscreen.h testframe.h textfilter.h \
			trayicon.h url.h util.h welcomescreen.h windowcontent.h zlibbuffer.h
PROXIMODO = distrib/proximodo

.phony: default all

default: all

all: $(PROXIMODO) tags

depend: $(SRC) $(HDR)
	makedepend -Y $(SRC)

clean:
	rm -f $(OBJ) $(PROXIMODO) tags

$(PROXIMODO): $(OBJ)
	$(CC) -o $@ $(OBJ) $(WXLIB)

.cpp.o:
	$(CC) $(CFLAGS) $(WXINC) -c -o $@ $<

tags: $(SRC) $(HDR)
	ctags -o $@ $(SRC) $(HDR)

# DO NOT DELETE THIS LINE -- make depend depends on it.

configscreen.o: configscreen.h controls.h windowcontent.h settings.h
configscreen.o: descriptor.h editscreen.h testframe.h mainframe.h events.h
configscreen.o: trayicon.h const.h util.h proxy.h requestmanager.h
configscreen.o: zlibbuffer.h receptor.h filterowner.h url.h textfilter.h
configscreen.o: filter.h memory.h matcher.h node.h headerfilter.h giffilter.h
configscreen.o: managerthread.h images/box_on.xpm images/box_off.xpm
configscreen.o: images/box_half.xpm images/btn_addfold32.xpm
configscreen.o: images/btn_addfilt32.xpm images/btn_trash32.xpm
configscreen.o: images/btn_edit32.xpm images/btn_ok32.xpm
configscreen.o: images/btn_undo32.xpm
controls.o: util.h controls.h events.h
descriptor.o: descriptor.h matcher.h node.h filter.h filterowner.h url.h
descriptor.o: memory.h util.h settings.h
editscreen.o: editscreen.h controls.h descriptor.h testframe.h matcher.h
editscreen.o: node.h filter.h filterowner.h url.h memory.h settings.h const.h
editscreen.o: util.h images/icon32.xpm
events.o: events.h
expander.o: expander.h filter.h filterowner.h url.h memory.h const.h util.h
expander.o: settings.h descriptor.h matcher.h node.h log.h events.h
expander.o: logframe.h windowcontent.h controls.h
filter.o: filter.h filterowner.h url.h memory.h log.h events.h logframe.h
filter.o: windowcontent.h settings.h descriptor.h controls.h
filterowner.o: filterowner.h url.h util.h
giffilter.o: giffilter.h receptor.h
headerfilter.o: headerfilter.h descriptor.h matcher.h node.h filter.h
headerfilter.o: filterowner.h url.h memory.h expander.h util.h log.h events.h
headerfilter.o: logframe.h windowcontent.h settings.h controls.h
log.o: log.h events.h logframe.h windowcontent.h settings.h descriptor.h
log.o: controls.h
logframe.o: logframe.h windowcontent.h settings.h descriptor.h events.h
logframe.o: controls.h images/icon32.xpm util.h
main.o: proxy.h requestmanager.h zlibbuffer.h receptor.h filterowner.h url.h
main.o: textfilter.h filter.h memory.h descriptor.h matcher.h node.h
main.o: headerfilter.h giffilter.h managerthread.h const.h settings.h
main.o: mainframe.h events.h windowcontent.h trayicon.h util.h
mainframe.o: mainframe.h events.h windowcontent.h settings.h descriptor.h
mainframe.o: trayicon.h proxy.h requestmanager.h zlibbuffer.h receptor.h
mainframe.o: filterowner.h url.h textfilter.h filter.h memory.h matcher.h
mainframe.o: node.h headerfilter.h giffilter.h managerthread.h log.h
mainframe.o: logframe.h controls.h util.h const.h welcomescreen.h
mainframe.o: settingsscreen.h configscreen.h editscreen.h testframe.h
mainframe.o: images/btn_settings.xpm images/btn_log.xpm images/btn_config.xpm
mainframe.o: images/btn_help.xpm images/btn_quit.xpm images/icon32.xpm
managerthread.o: managerthread.h requestmanager.h zlibbuffer.h receptor.h
managerthread.o: filterowner.h url.h textfilter.h filter.h memory.h
managerthread.o: descriptor.h matcher.h node.h headerfilter.h giffilter.h
managerthread.o: log.h events.h logframe.h windowcontent.h settings.h
managerthread.o: controls.h
matcher.o: matcher.h node.h filter.h filterowner.h url.h memory.h settings.h
matcher.o: descriptor.h nodes.h const.h util.h log.h events.h logframe.h
matcher.o: windowcontent.h controls.h
nodes.o: nodes.h node.h memory.h url.h matcher.h filter.h filterowner.h
nodes.o: const.h util.h settings.h descriptor.h expander.h log.h events.h
nodes.o: logframe.h windowcontent.h controls.h
platform.o: platform.h
proxy.o: proxy.h requestmanager.h zlibbuffer.h receptor.h filterowner.h url.h
proxy.o: textfilter.h filter.h memory.h descriptor.h matcher.h node.h
proxy.o: headerfilter.h giffilter.h managerthread.h util.h settings.h
proxy.o: events.h log.h logframe.h windowcontent.h controls.h platform.h
requestmanager.o: requestmanager.h zlibbuffer.h receptor.h filterowner.h
requestmanager.o: url.h textfilter.h filter.h memory.h descriptor.h matcher.h
requestmanager.o: node.h headerfilter.h giffilter.h util.h settings.h log.h
requestmanager.o: events.h logframe.h windowcontent.h controls.h const.h
settings.o: settings.h descriptor.h util.h const.h matcher.h node.h filter.h
settings.o: filterowner.h url.h memory.h
settingsscreen.o: settingsscreen.h controls.h windowcontent.h settings.h
settingsscreen.o: descriptor.h mainframe.h events.h trayicon.h util.h const.h
settingsscreen.o: proxy.h requestmanager.h zlibbuffer.h receptor.h
settingsscreen.o: filterowner.h url.h textfilter.h filter.h memory.h
settingsscreen.o: matcher.h node.h headerfilter.h giffilter.h managerthread.h
settingsscreen.o: images/btn_add20.xpm images/btn_view20.xpm
settingsscreen.o: images/btn_file20.xpm images/btn_trash20.xpm
testframe.o: testframe.h descriptor.h controls.h settings.h filterowner.h
testframe.o: url.h matcher.h node.h filter.h memory.h expander.h const.h
testframe.o: log.h events.h logframe.h windowcontent.h images/icon32.xpm
textfilter.o: textfilter.h filter.h filterowner.h url.h memory.h descriptor.h
textfilter.o: receptor.h matcher.h node.h expander.h const.h log.h events.h
textfilter.o: logframe.h windowcontent.h settings.h controls.h
trayicon.o: trayicon.h images/icon32.xpm settings.h descriptor.h log.h
trayicon.o: events.h logframe.h windowcontent.h controls.h proxy.h
trayicon.o: requestmanager.h zlibbuffer.h receptor.h filterowner.h url.h
trayicon.o: textfilter.h filter.h memory.h matcher.h node.h headerfilter.h
trayicon.o: giffilter.h managerthread.h mainframe.h util.h const.h
url.o: url.h
util.o: util.h platform.h
welcomescreen.o: welcomescreen.h windowcontent.h settings.h descriptor.h
welcomescreen.o: images/logo240.xpm
windowcontent.o: windowcontent.h settings.h descriptor.h util.h
zlibbuffer.o: zlibbuffer.h const.h
