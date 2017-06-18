#include "functionsdblockstranslator.h"

using json = nlohmann::json;

const std::string FunctionsdBlocksTranslator::ELECTROPHORER_STR = "Electrophorer";
const std::string FunctionsdBlocksTranslator::LIGHT_STR = "Ligth";
const std::string FunctionsdBlocksTranslator::HEATER_STR = "Heater";
const std::string FunctionsdBlocksTranslator::FLUORESCENCE_SENSOR_STR = "Fluorescence_sensor";
const std::string FunctionsdBlocksTranslator::OD_SENSOR_STR = "OD_sensor";
const std::string FunctionsdBlocksTranslator::LUMINISCENCE_SENSOR_STR = "Luminiscence_sensor";
const std::string FunctionsdBlocksTranslator::VOLUME_SENSOR_STR = "Volume_sensor";
const std::string FunctionsdBlocksTranslator::TEMPERATURE_SENSOR_STR = "Temperature_sensor";
const std::string FunctionsdBlocksTranslator::STIR_STR = "Stirer";
const std::string FunctionsdBlocksTranslator::SHAKE_STR = "Shaker";
const std::string FunctionsdBlocksTranslator::CENTRIFUGATE_STR = "Centrifugator";
const std::string FunctionsdBlocksTranslator::FUNCTION_LIST_STR = "functions_list";

const FunctionsdBlocksTranslator::FunctionsMap FunctionsdBlocksTranslator::functionsTypeMap(makeFunctionsMap());

FunctionsdBlocksTranslator::FunctionsMap FunctionsdBlocksTranslator::makeFunctionsMap() {
    FunctionsMap functions;

    functions.insert(std::make_pair(ELECTROPHORER_STR, FunctionsdBlocksTranslator::parseElectrophorerFunction));
    functions.insert(std::make_pair(LIGHT_STR, FunctionsdBlocksTranslator::parseLightFunction));
    functions.insert(std::make_pair(HEATER_STR, FunctionsdBlocksTranslator::parseHeatFunction));
    functions.insert(std::make_pair(FLUORESCENCE_SENSOR_STR, FunctionsdBlocksTranslator::parseFluorescenceSensorFunction));
    functions.insert(std::make_pair(OD_SENSOR_STR, FunctionsdBlocksTranslator::parseOdSensorFunction));
    functions.insert(std::make_pair(LUMINISCENCE_SENSOR_STR, FunctionsdBlocksTranslator::parseLuminiscenceSensorFunction));
    functions.insert(std::make_pair(VOLUME_SENSOR_STR, FunctionsdBlocksTranslator::parseVolumeSensorFunction));
    functions.insert(std::make_pair(TEMPERATURE_SENSOR_STR, FunctionsdBlocksTranslator::parseTemperatureSensorFunction));
    functions.insert(std::make_pair(STIR_STR, FunctionsdBlocksTranslator::parseStirFunction));
    functions.insert(std::make_pair(SHAKE_STR, FunctionsdBlocksTranslator::parseShakeFunction));
    functions.insert(std::make_pair(CENTRIFUGATE_STR, FunctionsdBlocksTranslator::parseCentrifugateFunction));

    return functions;
}

std::vector<std::shared_ptr<Function>> FunctionsdBlocksTranslator::processFunctions(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        std::vector<std::shared_ptr<Function>> functions;
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"type"}, functionObj);

        std::string typeStr = functionObj["type"];
        if (typeStr.compare(FUNCTION_LIST_STR) == 0) {
            UtilsJSON::checkPropertiesExists(std::vector<std::string>{"functionsList"}, functionObj);

            json functionList = functionObj["functionsList"];
            for(auto it = functionList.begin(); it != functionList.end(); ++it) {
                json actualFunction= *it;

                UtilsJSON::checkPropertiesExists(std::vector<std::string>{"type"}, actualFunction);
                std::string actualType = actualFunction["type"];

                functions.push_back(processSingleFunction(actualType, actualFunction));
            }
        } else {
            functions.push_back(processSingleFunction(typeStr, functionObj));
        }
        return functions;
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processFunctions. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<ValvePluginRouteFunction> FunctionsdBlocksTranslator::processValveFunction(
        const nlohmann::json & functionObj,
        ValveNode::TruthTable & truthTable)
    throw(std::invalid_argument)
{
    try {
        PluginConfiguration configObj = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"truthTable"}, functionObj);
        truthTable = parseTruthTable(functionObj["truthTable"]);

        return std::make_shared<ValvePluginRouteFunction>(std::shared_ptr<PluginAbstractFactory>(), configObj);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processValveFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<PumpPluginFunction> FunctionsdBlocksTranslator::processPumpFunction(
        const nlohmann::json & functionObj,
        bool & reversible,
        std::unordered_set<int> & portsIn,
        std::unordered_set<int> & portsOut)
    throw(std::invalid_argument)
{
    try {
        PluginConfiguration configObj = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "reversible",
                                             "inPorts",
                                             "outPorts"
                                         }, functionObj);

        reversible = functionObj["reversible"];

        json inPortsList = functionObj["inPorts"];
        for(auto it = inPortsList.begin(); it != inPortsList.end(); ++it) {
            int actualInPort = *it;
            portsIn.insert(actualInPort-1);
        }

        json outPortsList = functionObj["outPorts"];
        for(auto it = outPortsList.begin(); it != outPortsList.end(); ++it) {
            int actualOutPort = *it;
            portsOut.insert(actualOutPort-1);
        }

        PumpWorkingRange wRange = parsePumpWorkingRange(functionObj);
        return std::make_shared<PumpPluginFunction>(std::shared_ptr<PluginAbstractFactory>(), configObj, wRange);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processPumpFunction. Exception ocurred " + std::string(e.what())));
    }
}

void FunctionsdBlocksTranslator::processOpenGlasswareFunction(
        const nlohmann::json & functionObj,
        units::Volume & minVolume,
        units::Volume & maxVolume)
    throw(std::invalid_argument)
{
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minVolume",
                                             "minVolumeUnits",
                                             "maxVolume",
                                             "maxVolumeUnits"
                                         }, functionObj);

       double minVolumeValue = functionObj["minVolume"];
       minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

       double maxVolumeValue = functionObj["maxVolume"];
       maxVolume = maxVolumeValue * UtilsJSON::getVolumeUnits(functionObj["maxVolumeUnits"]);

    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processOpenGlasswareFunction. Exception ocurred " + std::string(e.what())));
    }
}

void FunctionsdBlocksTranslator::processCloseGlasswareFunction(
        const nlohmann::json & functionObj,
        units::Volume & minVolume,
        units::Volume & maxVolume,
        std::unordered_set<int> & portsIn,
        std::unordered_set<int> & portsOut)
    throw(std::invalid_argument)
{
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minVolume",
                                             "minVolumeUnits",
                                             "maxVolume",
                                             "maxVolumeUnits",
                                             "inPorts",
                                             "outPorts"
                                         }, functionObj);

       double minVolumeValue = functionObj["minVolume"];
       minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

       double maxVolumeValue = functionObj["maxVolume"];
       maxVolume = maxVolumeValue * UtilsJSON::getVolumeUnits(functionObj["maxVolumeUnits"]);

       json inPortsList = functionObj["inPorts"];
       for(auto it = inPortsList.begin(); it != inPortsList.end(); ++it) {
           int actualInPort = *it;
           portsIn.insert(actualInPort-1);
       }

       json outPortsList = functionObj["outPorts"];
       for(auto it = outPortsList.begin(); it != outPortsList.end(); ++it) {
           int actualOutPort = *it;
           portsOut.insert(actualOutPort-1);
       }

    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processCloseGlasswareFunction. Exception ocurred " + std::string(e.what())));
    }
}

PluginConfiguration FunctionsdBlocksTranslator::fillConfigurationObj(const nlohmann::json & pluginObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "block_type",
                                             "type",
                                             "paramsNumber"}, pluginObj);
        std::string name = pluginObj["block_type"];
        std::string pluginType = pluginObj["type"];

        int paramsNumber = pluginObj["paramsNumber"];
        std::unordered_map<std::string,std::string> params;
        for(int i=0; i < paramsNumber; i++) {
            std::string actualName = "name" + std::to_string(i);
            std::string actualValue = "value" + std::to_string(i);

            UtilsJSON::checkPropertiesExists(std::vector<std::string>{actualName, actualValue}, pluginObj);

            std::string nameStr = pluginObj[actualName];
            std::string valueStr = InputsBlocksTranslator::processInput(pluginObj[actualValue]);
            params.insert(std::make_pair(nameStr, valueStr));
        }

        return PluginConfiguration(name, pluginType, params);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::fillConfigurationObj. Exception ocurred " + std::string(e.what())));
    }
}

ValveNode::TruthTable FunctionsdBlocksTranslator::parseTruthTable(const nlohmann::json & truthTableObj)
    throw(std::invalid_argument)
{
    try {
        ValveNode::TruthTable tTable;
        for(auto it = truthTableObj.begin(); it != truthTableObj.end(); ++it) {
            json row = *it;
            UtilsJSON::checkPropertiesExists(std::vector<std::string>{"position", "connected_pins"}, row);

            int position = row["position"];
            std::vector<std::unordered_set<int>> connectedPins = parseConnectedPins(row["connected_pins"]);

            tTable.insert(std::make_pair(position, connectedPins));
        }
        return tTable;
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseTruthTable. Exception ocurred " + std::string(e.what())));
    }
}

std::vector<std::unordered_set<int>> FunctionsdBlocksTranslator::parseConnectedPins(const nlohmann::json & connectedPins) {
    std::vector<std::unordered_set<int>> connectedPinsVector;
    for(auto it = connectedPins.begin(); it != connectedPins.end(); ++it) {
        json connectedPinsElem = *it;
        std::unordered_set<int> connectedPinsSet;

        for(auto itElem = connectedPinsElem.begin(); itElem != connectedPinsElem.end(); ++itElem) {
            int elemPin = *itElem;
            connectedPinsSet.insert(elemPin);
        }
        connectedPinsVector.push_back(connectedPinsSet);
    }
    return connectedPinsVector;
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::processSingleFunction(const std::string & typeStr, const nlohmann::json & functionObj) throw(std::invalid_argument) {
    std::shared_ptr<Function> actualFunction;

    auto finded = functionsTypeMap.find(typeStr);
    if (finded != functionsTypeMap.end()) {
        std::function<std::shared_ptr<Function>(const nlohmann::json &)> typeFunction = finded->second;
        actualFunction = typeFunction(functionObj);
    } else {
        throw(std::invalid_argument("unknow type: " + typeStr));
    }
    return actualFunction;
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseElectrophorerFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        ElectrophoresisWorkingRange range = parseElectrophoresisWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<ElectrophoresisFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseElectrophorerFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseLightFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        LigthWorkingRange range = parseLghtsWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<LightFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseLightFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseHeatFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        HeaterWorkingRange range = parseHeatersWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<HeatFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseHeatFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseFluorescenceSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        MeasureFluorescenceWorkingRange range = parseMeasureFluorescenceWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<MeasureFluorescenceFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseFluorescenceSensorFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseOdSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        MeasureOdWorkingRange range = parseMeasureOdWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<MeasureOdFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseOdSensorFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseLuminiscenceSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<MeasureLuminiscenceFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseLuminiscenceSensorFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseVolumeSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<MeasureVolumeFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseVolumeSensorFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseTemperatureSensorFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<MeasureTemperatureFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseTemperatureSensorFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseStirFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        StirWorkingRange range = parseStirWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<StirFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseStirFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseShakeFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        ShakeWorkingRange range = parseShakerWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<ShakeFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseShakeFunction. Exception ocurred " + std::string(e.what())));
    }
}

std::shared_ptr<Function> FunctionsdBlocksTranslator::parseCentrifugateFunction(const nlohmann::json & functionObj) throw(std::invalid_argument) {
    try {
        PluginConfiguration configuration = fillConfigurationObj(functionObj);
        CentrifugationWorkingRange range = parseCentrifugationWorkingRange(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"minVolume", "minVolumeUnits"}, functionObj);

        double minVolumeValue = functionObj["minVolume"];
        units::Volume minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

        return std::make_shared<CentrifugateFunction>(std::shared_ptr<PluginAbstractFactory>(),configuration, minVolume, range);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseCentrifugateFunction. Exception ocurred " + std::string(e.what())));
    }
}

CentrifugationWorkingRange FunctionsdBlocksTranslator::parseCentrifugationWorkingRange(const nlohmann::json & centrifugateObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeUnits",
                                             "maxRange",
                                             "maxRangeUnits"}, centrifugateObj);

        double minRateValue = centrifugateObj["minRange"];
        units::Frequency minRate = minRateValue * UtilsJSON::getFrequencyUnits(centrifugateObj["minRangeUnits"]);

        double maxRateValue = centrifugateObj["maxRange"];
        units::Frequency maxRate = maxRateValue * UtilsJSON::getFrequencyUnits(centrifugateObj["maxRangeUnits"]);

        return CentrifugationWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseCentrifugationWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

ElectrophoresisWorkingRange FunctionsdBlocksTranslator::parseElectrophoresisWorkingRange(const nlohmann::json & electrophorerObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRageEFieldUnits",
                                             "minRageLengthUnits",
                                             "maxRange",
                                             "maxRageEFieldUnits",
                                             "maxRageLengthUnits"}, electrophorerObj);

        double minRateValue = electrophorerObj["minRange"];
        units::ElectricField minRate = minRateValue *
                (UtilsJSON::getElectricPotentialUnits(electrophorerObj["minRageEFieldUnits"]) / UtilsJSON::getLengthUnits(electrophorerObj["minRageLengthUnits"]));

        double maxRateValue = electrophorerObj["maxRange"];
        units::ElectricField maxRate = maxRateValue *
                (UtilsJSON::getElectricPotentialUnits(electrophorerObj["maxRageEFieldUnits"]) / UtilsJSON::getLengthUnits(electrophorerObj["maxRageLengthUnits"]));

        return ElectrophoresisWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseElectrophoresisWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

HeaterWorkingRange FunctionsdBlocksTranslator::parseHeatersWorkingRange(const nlohmann::json & heaterObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeUnits",
                                             "maxRange",
                                             "maxRangeUnits"}, heaterObj);

        double minRateValue = heaterObj["minRange"];
        units::Temperature minRate = minRateValue * UtilsJSON::getTemperatureUnits(heaterObj["minRangeUnits"]);

        double maxRateValue = heaterObj["maxRange"];
        units::Temperature maxRate = maxRateValue * UtilsJSON::getTemperatureUnits(heaterObj["maxRangeUnits"]);

        return HeaterWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseHeatersWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

LigthWorkingRange FunctionsdBlocksTranslator::parseLghtsWorkingRange(const nlohmann::json & lightObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minWavelength",
                                             "minWavelengthUnits",
                                             "maxWavelength",
                                             "maxWavelengthUnits",
                                             "minIntensity",
                                             "minIntensityUnits",
                                             "maxIntensity",
                                             "maxIntensityUnits"}, lightObj);

        double minWavelengthValue = lightObj["minWavelength"];
        units::Length minWavelength = minWavelengthValue * UtilsJSON::getLengthUnits(lightObj["minWavelengthUnits"]);

        double maxWavelengthValue = lightObj["maxWavelength"];
        units::Length maxWavelength = maxWavelengthValue * UtilsJSON::getLengthUnits(lightObj["maxWavelengthUnits"]);

        double minIntensityValue = lightObj["minIntensity"];
        units::LuminousIntensity minIntensity = minIntensityValue * UtilsJSON::getLuminousIntensityUnits(lightObj["minIntensityUnits"]);

        double maxIntensityValue = lightObj["maxIntensity"];
        units::LuminousIntensity maxIntensity = maxIntensityValue * UtilsJSON::getLuminousIntensityUnits(lightObj["maxIntensityUnits"]);

        return LigthWorkingRange(minWavelength, maxWavelength, minIntensity, maxIntensity);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseLghtsWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

MeasureFluorescenceWorkingRange FunctionsdBlocksTranslator::parseMeasureFluorescenceWorkingRange(const nlohmann::json & measureFluorescenceObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minEmission",
                                             "minEmissionUnits",
                                             "maxEmission",
                                             "maxEmissionUnits",
                                             "minExcitation",
                                             "minExcitationUnits",
                                             "maxExcitation",
                                             "maxExcitationUnits"}, measureFluorescenceObj);

        double minEmissionValue = measureFluorescenceObj["minEmission"];
        units::Length minEmission = minEmissionValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["minEmissionUnits"]);

        double maxEmissionValue = measureFluorescenceObj["maxEmission"];
        units::Length maxEmission = maxEmissionValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["maxEmissionUnits"]);

        double minExcitationValue = measureFluorescenceObj["minExcitation"];
        units::Length minExcitation = minExcitationValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["minExcitationUnits"]);

        double maxExcitationValue = measureFluorescenceObj["maxExcitation"];
        units::Length maxExcitation = maxExcitationValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["maxExcitationUnits"]);

        return MeasureFluorescenceWorkingRange(minEmission, maxEmission, minExcitation, maxExcitation);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseMeasureFluorescenceWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

MeasureOdWorkingRange FunctionsdBlocksTranslator::parseMeasureOdWorkingRange(const nlohmann::json & measureOdObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeUnits",
                                             "maxRange",
                                             "maxRangeUnits"}, measureOdObj);

        double minRateValue = measureOdObj["minRange"];
        units::Length minRate = minRateValue * UtilsJSON::getLengthUnits(measureOdObj["minRangeUnits"]);

        double maxRateValue = measureOdObj["maxRange"];
        units::Length maxRate = maxRateValue * UtilsJSON::getLengthUnits(measureOdObj["maxRangeUnits"]);

        return MeasureOdWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseMeasureOdWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

PumpWorkingRange FunctionsdBlocksTranslator::parsePumpWorkingRange(const nlohmann::json & pumpObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeVolumeUnits",
                                             "minRangeTimeUnits",
                                             "maxRange",
                                             "maxRangeVolumeUnits",
                                             "maxRangeTimeUnits"}, pumpObj);

        double minRateValue = pumpObj["minRange"];
        units::Volumetric_Flow minRate = minRateValue * (UtilsJSON::getVolumeUnits(pumpObj["minRangeVolumeUnits"]) / UtilsJSON::getTimeUnits(pumpObj["minRangeTimeUnits"]));

        double maxRateValue = pumpObj["maxRange"];
        units::Volumetric_Flow maxRate = maxRateValue * (UtilsJSON::getVolumeUnits(pumpObj["maxRangeVolumeUnits"]) / UtilsJSON::getTimeUnits(pumpObj["maxRangeTimeUnits"]));

        return PumpWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parsePumpWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

ShakeWorkingRange FunctionsdBlocksTranslator::parseShakerWorkingRange(const nlohmann::json & shakerObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeUnits",
                                             "maxRange",
                                             "maxRangeUnits"}, shakerObj);

        double minRateValue = shakerObj["minRange"];
        units::Frequency minRate = minRateValue * UtilsJSON::getFrequencyUnits(shakerObj["minRangeUnits"]);

        double maxRateValue = shakerObj["maxRange"];
        units::Frequency maxRate = maxRateValue * UtilsJSON::getFrequencyUnits(shakerObj["maxRangeUnits"]);

        return ShakeWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseShakerWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}

StirWorkingRange FunctionsdBlocksTranslator::parseStirWorkingRange(const nlohmann::json & stirObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeUnits",
                                             "maxRange",
                                             "maxRangeUnits"}, stirObj);

        double minRateValue = stirObj["minRange"];
        units::Frequency minRate = minRateValue * UtilsJSON::getFrequencyUnits(stirObj["minRangeUnits"]);

        double maxRateValue = stirObj["maxRange"];
        units::Frequency maxRate = maxRateValue * UtilsJSON::getFrequencyUnits(stirObj["maxRangeUnits"]);

        return StirWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseStirWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}













































