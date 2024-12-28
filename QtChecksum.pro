QT       += core gui network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 static

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp    \
    mainwindow.cpp \
    qckhash.cpp \
    qcksettingsdialog.cpp \
    qtapp.cpp \
    taskmanager.cpp \
    utils.cpp \
    workerthread.cpp \


HEADERS += \
    mainwindow.h \
    qckhash.h \
    qcksettingsdialog.h \
    qdocktile.h \
    qtapp.h \
    taskmanager.h \
    utils.h \
    workerthread.h

VERSION=1.0.3
VERSION_CODE=5
DEFINES += VERSION=\\\"$${VERSION}\\\" VERSION_CODE="$${VERSION_CODE}"
macos:{
LIBS += -framework Foundation -framework cocoa
SOURCES += \
    macOS/DockCircularProgressBar.mm \
    macOS/appdelegate.mm \
    macOS/qdocktile.mm \
    macOS/utils_osx.mm \


HEADERS += \
    macOS/appdelegate.h \
    macOS/DockCircularProgressBar.h \

QMAKE_INFO_PLIST = macOS/qthash.plist
ICON = macOS/icon.icns
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8

TRANSLATE_zh.files = macOS/zh.lproj/InfoPlist.strings macOS/zh.lproj/locversion.plist
TRANSLATE_zh.path = Contents/Resources/zh.lproj
QMAKE_BUNDLE_DATA += TRANSLATE_zh

}else{

SOURCES += \
    qdocktile.cpp \

}

win32:{
## OpenSSL Download from http://slproweb.com/download/Win32OpenSSL_Light-1_1_1g.msi
LIBS += -LC:/OpenSSL-Win32
QMAKE_TARGET_COPYRIGHT = Copyright 2024 WeiKeting. All rights reserved.
RC_ICONS = windows/icon.ico
#RESOURCES += win.qrc
}

FORMS += \
    mainwindow.ui \
    qck_settings_dialog.ui

TRANSLATIONS += \
    QtChecksum_zh.ts



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qthash.qrc

DISTFILES += \
    macOS/build_dmg.sh

