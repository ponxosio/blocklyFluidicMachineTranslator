#-------------------------------------------------
#
# Project created by QtCreator 2017-06-05T10:34:34
#
#-------------------------------------------------

# ensure one "debug_and_release" in CONFIG, for clarity...
debug_and_release {
    CONFIG -= debug_and_release
    CONFIG += debug_and_release
}
    # ensure one "debug" or "release" in CONFIG so they can be used as
    #   conditionals instead of writing "CONFIG(debug, debug|release)"...
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

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
    blocklyFluidicMachineTranslator/blocklyfluidicmachinetranslator_global.h \
    blocklyFluidicMachineTranslator/blocks/functionsdblockstranslator.h \
    blocklyFluidicMachineTranslator/blocks/inputsblockstranslator.h

SOURCES += \
    blocklyFluidicMachineTranslator/blocklyfluidicmachinetranslator.cpp \
    blocklyFluidicMachineTranslator/blocks/functionsdblockstranslator.cpp \
    blocklyFluidicMachineTranslator/blocks/inputsblockstranslator.cpp

debug {
    QMAKE_POST_LINK=X:\blockly_fluidicMachine_translator\blocklyFluidicMachineTranslator\setDLL.bat $$shell_path($$OUT_PWD/debug) debug

    INCLUDEPATH += X:\fluidicMachineModel\dll_debug\include
    LIBS += -L$$quote(X:\fluidicMachineModel\dll_debug\bin) -lFluidicMachineModel

    INCLUDEPATH += X:\constraintsEngine\dll_debug\include
    LIBS += -L$$quote(X:\constraintsEngine\dll_debug\bin) -lconstraintsEngineLibrary

    INCLUDEPATH += X:\utils\dll_debug\include
    LIBS += -L$$quote(X:\utils\dll_debug\bin) -lutils

    INCLUDEPATH += X:\commomModel\dll_debug\include
    LIBS += -L$$quote(X:\commomModel\dll_debug\bin) -lcommonModel

    INCLUDEPATH += X:\protocolGraph\dll_debug\include
    LIBS += -L$$quote(X:\protocolGraph\dll_debug\bin) -lprotocolGraph
}

!debug {
    QMAKE_POST_LINK=X:\blockly_fluidicMachine_translator\blocklyFluidicMachineTranslator\setDLL.bat $$shell_path($$OUT_PWD/release) release

    INCLUDEPATH += X:\fluidicMachineModel\dll_release\include
    LIBS += -L$$quote(X:\fluidicMachineModel\dll_release\bin) -lFluidicMachineModel

    INCLUDEPATH +=X:\constraintsEngine\dll_release\include
    LIBS += -L$$quote(X:\constraintsEngine\dll_release\bin) -lconstraintsEngineLibrary

    INCLUDEPATH += X:\utils\dll_release\include
    LIBS += -L$$quote(X:\utils\dll_release\bin) -lutils

    INCLUDEPATH += X:\commomModel\dll_release\include
    LIBS += -L$$quote(X:\commomModel\dll_release\bin) -lcommonModel

    INCLUDEPATH += X:\protocolGraph\dll_release\include
    LIBS += -L$$quote(X:\protocolGraph\dll_release\bin) -lprotocolGraph
}

INCLUDEPATH += X:\libraries\cereal-1.2.2\include
INCLUDEPATH += X:\libraries\json-2.1.1\src

INCLUDEPATH += X:\swipl\include
LIBS += -L$$quote(X:\swipl\bin) -llibswipl
LIBS += -L$$quote(X:\swipl\lib) -llibswipl

