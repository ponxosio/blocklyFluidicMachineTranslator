COPY %1\blocklyFluidicMachineTranslator.dll X:\blockly_fluidicMachine_translator\dll_%2\bin\blocklyFluidicMachineTranslator.dll
COPY %1\blocklyFluidicMachineTranslator.lib X:\blockly_fluidicMachine_translator\dll_%2\bin\blocklyFluidicMachineTranslator.lib

DEL /S X:\blockly_fluidicMachine_translator\dll_%2\include\*.h
XCOPY /S /Y X:\blockly_fluidicMachine_translator\blocklyFluidicMachineTranslator\*.h X:\blockly_fluidicMachine_translator\dll_%2\include