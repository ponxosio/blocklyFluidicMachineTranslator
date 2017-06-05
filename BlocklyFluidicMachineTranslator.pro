#-------------------------------------------------
#
# Project created by QtCreator 2017-06-05T10:34:34
#
#-------------------------------------------------

QT       -= gui

TARGET = blocklyFluidicMachineTranslator
TEMPLATE = lib

DEFINES += BLOCKLYFLUIDICMACHINETRANSLATOR_LIBRARY

unix {
    target.path = /usr/lib
    INSTALLS += target
}

HEADERS += \
    blocklyFluidicMachineTranslator/blocklyfluidicmachinetranslator.h \
    blocklyFluidicMachineTranslator/blocklyfluidicmachinetranslator_global.h

SOURCES += \
    blocklyFluidicMachineTranslator/blocklyfluidicmachinetranslator.cpp

debug {
    QMAKE_POST_LINK=X:\blockly_fluidicMachine_translator\blocklyFluidicMachineTranslator\setDLL.bat $$shell_path($$OUT_PWD/debug) debug

    INCLUDEPATH += X:\utils\dll_debug\include
    LIBS += -L$$quote(X:\utils\dll_debug\bin) -lutils

    INCLUDEPATH += X:\fluidicMachineModel\dll_debug\include
    LIBS += -L$$quote(X:\fluidicMachineModel\dll_debug\bin) -lFluidicMachineModel

    INCLUDEPATH += X:\commomModel\dll_debug\include
    LIBS += -L$$quote(X:\commomModel\dll_debug\bin) -lcommonModel
}

!debug {
    QMAKE_POST_LINK=X:\blockly_fluidicMachine_translator\blocklyFluidicMachineTranslator\setDLL.bat $$shell_path($$OUT_PWD/release) release

    INCLUDEPATH += X:\utils\dll_release\include
    LIBS += -L$$quote(X:\utils\dll_release\bin) -lutils

    INCLUDEPATH += X:\fluidicMachineModel\dll_release\include
    LIBS += -L$$quote(X:\fluidicMachineModel\dll_release\bin) -lFluidicMachineModel

    INCLUDEPATH += X:\commomModel\dll_release\include
    LIBS += -L$$quote(X:\commomModel\dll_release\bin) -lcommonModel
}

INCLUDEPATH += X:\libraries\json-2.1.1\src
INCLUDEPATH += X:\libraries\boost_1_63_0
INCLUDEPATH += X:\libraries\cereal-1.2.2\include
