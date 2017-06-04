QT -= gui

# prepare build
TARGET = telegrambotlib-qt
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = lib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Include telegrambotlib-qt
include(telegrambotlib-qt.pri)

# Binary installer
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

# Header installer
header.files = $$HEADERS
header.path = $$[QT_INSTALL_HEADERS]/$$TARGET
INSTALLS += header

# in debug mode append a d to target binary name
CONFIG(debug, debug|release) {
    TARGET = $${TARGET}"d"
}
CONFIG(release, debug|release) {
    TARGET = $${TARGET}
}

# inform user
!build_pass:message("library headers install target folder: "$$header.path"/")
!build_pass:message("library install target: "$$target.path"/"$${TARGET}.$${QMAKE_EXTENSION_SHLIB})