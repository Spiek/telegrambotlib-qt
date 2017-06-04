QT += core network
CONFIG += c++14

SOURCES +=	$$PWD/src/telegrambot.cpp \
			$$PWD/src/jsonhelper.cpp \
			$$PWD/modules/sslserver/sslserver.cpp \
			$$PWD/modules/httpserver/httpserver.cpp

HEADERS +=	$$PWD/src/telegrambot.h \
			$$PWD/src/jsonhelper.h \
			$$PWD/src/telegramdatastructs.h \
			$$PWD/src/telegramdatainterface.h \
			$$PWD/modules/sslserver/sslserver.h \
			$$PWD/modules/httpserver/httpserver.h

INCLUDEPATH += $$PWD/src/
INCLUDEPATH += $$PWD/modules/sslserver/
INCLUDEPATH += $$PWD/modules/httpserver/

#include QDelegate dependency
include($$PWD/vendor/qdelegate/QDelegate.pri)
