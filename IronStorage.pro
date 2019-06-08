QT += quick quickcontrols2
CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    include/filedisk.h \
    src/mounteddiskinfo.h \
    src/qmlfacade.h \

SOURCES += \
        src/main.cpp \
        src/mounteddiskinfo.cpp \
        src/qmlfacade.cpp

RESOURCES += src/qml.qrc \
        src/resources.qrc

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/ -lFileDiskLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/ -lFileDiskLibd

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/libFileDiskLib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/libFileDiskLibd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/FileDiskLib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/FileDiskLibd.lib

DISTFILES +=
