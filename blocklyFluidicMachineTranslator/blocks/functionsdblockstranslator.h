#ifndef FUNCTIONSDBLOCKSTRANSLATOR_H
#define FUNCTIONSDBLOCKSTRANSLATOR_H

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <json.hpp>

#include <commonmodel/functions/function.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>
#include <commonmodel/functions/electrophoresisfunction.h>
#include <commonmodel/functions/lightfunction.h>
#include <commonmodel/functions/heatfunction.h>
#include <commonmodel/functions/measurefluorescencefunction.h>
#include <commonmodel/functions/measureodfunction.h>
#include <commonmodel/functions/measureluminiscencefunction.h>
#include <commonmodel/functions/measurevolumefunction.h>
#include <commonmodel/functions/measuretemperaturefunction.h>
#include <commonmodel/functions/stirfunction.h>
#include <commonmodel/functions/shakefunction.h>
#include <commonmodel/functions/centrifugatefunction.h>

#include <commonmodel/functions/ranges/centrifugationworkingrange.h>
#include <commonmodel/functions/ranges/electrophoresisworkingrange.h>
#include <commonmodel/functions/ranges/emptyworkingrange.h>
#include <commonmodel/functions/ranges/heaterworkingrange.h>
#include <commonmodel/functions/ranges/ligthworkingrange.h>
#include <commonmodel/functions/ranges/measurefluorescenceworkingrange.h>
#include <commonmodel/functions/ranges/measureodworkingrange.h>
#include <commonmodel/functions/ranges/pumpworkingrange.h>
#include <commonmodel/functions/ranges/shakeworkingrange.h>
#include <commonmodel/functions/ranges/stirworkingrange.h>

#include <fluidicmachinemodel/fluidicnode/valvenode.h>

#include <utils/utils.h>
#include <utils/utilsjson.h>

#include "blocklyFluidicMachineTranslator/blocks/inputsblockstranslator.h"

class FunctionsdBlocksTranslator
{
    typedef std::unordered_map<std::string, std::function<std::shared_ptr<Function>(const nlohmann::json &)>> FunctionsMap;

    static const std::string ELECTROPHORER_STR;
    static const std::string LIGHT_STR;
    static const std::string HEATER_STR;
    static const std::string FLUORESCENCE_SENSOR_STR;
    static const std::string OD_SENSOR_STR;
    static const std::string LUMINISCENCE_SENSOR_STR;
    static const std::string VOLUME_SENSOR_STR;
    static const std::string TEMPERATURE_SENSOR_STR;
    static const std::string STIR_STR;
    static const std::string SHAKE_STR;
    static const std::string CENTRIFUGATE_STR;
    static const std::string FUNCTION_LIST_STR;

    static const FunctionsMap functionsTypeMap;

    static FunctionsMap makeFunctionsMap();

public:
    virtual ~FunctionsdBlocksTranslator(){}

    static std::vector<std::shared_ptr<Function>> processFunctions(const nlohmann::json & functionObj) throw(std::invalid_argument);

    static std::shared_ptr<ValvePluginRouteFunction> processValveFunction(const nlohmann::json & functionObj,
                                                                          ValveNode::TruthTable & truthTable) throw(std::invalid_argument);

    static std::shared_ptr<PumpPluginFunction> processPumpFunction(const nlohmann::json & functionObj, bool & reversible) throw(std::invalid_argument);

    static void processOpenGlasswareFunction(const nlohmann::json & functionObj,
                                             units::Volume & minVolume,
                                             units::Volume & maxVolume) throw(std::invalid_argument);

    static void processCloseGlasswareFunction(const nlohmann::json & functionObj,
                                              units::Volume & minVolume,
                                              units::Volume & maxVolume) throw(std::invalid_argument);

protected:
    static PluginConfiguration fillConfigurationObj(const nlohmann::json & pluginObj) throw(std::invalid_argument);

    static ValveNode::TruthTable parseTruthTable(const nlohmann::json & truthTableObj) throw(std::invalid_argument);
    static std::vector<std::unordered_set<int>> parseConnectedPins(const nlohmann::json & connectedPins);

    static std::shared_ptr<Function> processSingleFunction(const std::string & typeStr, const nlohmann::json & functionObj) throw(std::invalid_argument);

    static std::shared_ptr<Function> parseElectrophorerFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseLightFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseHeatFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseFluorescenceSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseOdSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseLuminiscenceSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseVolumeSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseTemperatureSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseStirFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseShakeFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);
    static std::shared_ptr<Function> parseCentrifugateFunction(const nlohmann::json & functionObj) throw(std::invalid_argument);

    static CentrifugationWorkingRange parseCentrifugationWorkingRange(const nlohmann::json & centrifugateObj) throw(std::invalid_argument);
    static ElectrophoresisWorkingRange parseElectrophoresisWorkingRange(const nlohmann::json & electrophorerObj) throw(std::invalid_argument);
    static HeaterWorkingRange parseHeatersWorkingRange(const nlohmann::json & heaterObj) throw(std::invalid_argument);
    static LigthWorkingRange parseLghtsWorkingRange(const nlohmann::json & lightObj) throw(std::invalid_argument);
    static MeasureFluorescenceWorkingRange parseMeasureFluorescenceWorkingRange(const nlohmann::json & measureFluorescenceObj) throw(std::invalid_argument);
    static MeasureOdWorkingRange parseMeasureOdWorkingRange(const nlohmann::json & measureOdObj) throw(std::invalid_argument);
    static PumpWorkingRange parsePumpWorkingRange(const nlohmann::json & pumpObj) throw(std::invalid_argument);
    static ShakeWorkingRange parseShakerWorkingRange(const nlohmann::json & shakerObj) throw(std::invalid_argument);
    static StirWorkingRange parseStirWorkingRange(const nlohmann::json & stirObj) throw(std::invalid_argument);
};

#endif // FUNCTIONSDBLOCKSTRANSLATOR_H
