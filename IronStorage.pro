QT += quick quickcontrols2
CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    include/IronStorage.h \
    src/diskinfomodel.h \
    src/diskmanager.h \
    src/mounteddiskinfo.h \
    src/qmlfacade.h \

SOURCES += \
        src/diskinfomodel.cpp \
        src/diskmanager.cpp \
        src/main.cpp \
        src/mounteddiskinfo.cpp \
        src/qmlfacade.cpp

RESOURCES += src/qml.qrc \
             resources/resources.qrc \
             translations/translations.qrc

TRANSLATIONS += \
    translation/en.ts \
    translation/ru.ts \
    translation/ua.ts

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/ -lIronStorageInterface
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/ -lIronStorageInterface

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/IronStorageInterface.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/IronStorageInterface.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/IronStorageInterface.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/IronStorageInterface.lib

DISTFILES +=
