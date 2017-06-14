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
        if (typeStr.compare(FUNCTION_LIST_STR)) {
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
        bool & reversible)
    throw(std::invalid_argument)
{
    try {
        PluginConfiguration configObj = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"reversible"}, functionObj);
        reversible = functionObj["reversible"];

        PumpWorkingRange wRange = parsePumpWorkingRange(functionObj);

        return std::make_shared<PumpPluginFunction>(std::shared_ptr<PluginAbstractFactory>(), configObj, wRange);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processPumpFunction. Exception ocurred " + std::string(e.what())));
    }
}

void FunctionsdBlocksTranslator::processGlasswareFunction(
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

       std::string minVolumeStr = functionObj["minVolume"];
       double minVolumeValue = std::atof(minVolumeStr.c_str());
       minVolume = minVolumeValue * UtilsJSON::getVolumeUnits(functionObj["minVolumeUnits"]);

       std::string maxVolumeStr = functionObj["maxVolume"];
       double maxVolumeValue = std::atof(maxVolumeStr.c_str());
       maxVolume = maxVolumeValue * UtilsJSON::getVolumeUnits(functionObj["maxVolumeUnits"]);

    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::processGlasswareFunction. Exception ocurred " + std::string(e.what())));
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

CentrifugationWorkingRange FunctionsdBlocksTranslator::parseCentrifugationWorkingRange(const nlohmann::json & centrifugateObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeUnits",
                                             "maxRange",
                                             "maxRangeUnits"}, centrifugateObj);

        std::string minRateStr = centrifugateObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::Frequency minRate = minRateValue * UtilsJSON::getFrequencyUnits(centrifugateObj["minRangeUnits"]);

        std::string maxRateStr = centrifugateObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
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

        std::string minRateStr = electrophorerObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::ElectricField minRate = minRateValue *
                (UtilsJSON::getElectricPotentialUnits(electrophorerObj["minRageEFieldUnits"]) / UtilsJSON::getLengthUnits(electrophorerObj["minRageLengthUnits"]));

        std::string maxRateStr = electrophorerObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
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

        std::string minRateStr = heaterObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::Temperature minRate = minRateValue * UtilsJSON::getTemperatureUnits(heaterObj["minRangeUnits"]);

        std::string maxRateStr = heaterObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
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

        std::string minWavelengthStr = lightObj["minWavelength"];
        double minWavelengthValue = std::atof(minWavelengthStr.c_str());
        units::Length minWavelength = minWavelengthValue * UtilsJSON::getLengthUnits(lightObj["minWavelengthUnits"]);

        std::string maxWavelengthStr = lightObj["maxWavelength"];
        double maxWavelengthValue = std::atof(maxWavelengthStr.c_str());
        units::Length maxWavelength = maxWavelengthValue * UtilsJSON::getLengthUnits(lightObj["maxWavelengthUnits"]);

        std::string minIntensityStr = lightObj["minIntensity"];
        double minIntensityValue = std::atof(minIntensityStr.c_str());
        units::LuminousIntensity minIntensity = minIntensityValue * UtilsJSON::getLuminousIntensityUnits(lightObj["minIntensityUnits"]);

        std::string maxIntensityStr = lightObj["maxIntensity"];
        double maxIntensityValue = std::atof(maxIntensityStr.c_str());
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

        std::string minEmissionStr = measureFluorescenceObj["minEmission"];
        double minEmissionValue = std::atof(minEmissionStr.c_str());
        units::Length minEmission = minEmissionValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["minEmissionUnits"]);

        std::string maxEmissionStr = measureFluorescenceObj["maxEmission"];
        double maxEmissionValue = std::atof(maxEmissionStr.c_str());
        units::Length maxEmission = maxEmissionValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["maxEmissionUnits"]);

        std::string minExcitationStr = measureFluorescenceObj["minExcitation"];
        double minExcitationValue = std::atof(minExcitationStr.c_str());
        units::Length minExcitation = minExcitationValue * UtilsJSON::getLengthUnits(measureFluorescenceObj["minExcitationUnits"]);

        std::string maxExcitationStr = measureFluorescenceObj["maxExcitation"];
        double maxExcitationValue = std::atof(maxExcitationStr.c_str());
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

        std::string minRateStr = measureOdObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::Length minRate = minRateValue * UtilsJSON::getLengthUnits(measureOdObj["minRangeUnits"]);

        std::string maxRateStr = measureOdObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
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

        std::string minRateStr = pumpObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::Volumetric_Flow minRate = minRateValue * (UtilsJSON::getVolumeUnits(pumpObj["minRangeVolumeUnits"]) / UtilsJSON::getTimeUnits(pumpObj["minRangeTimeUnits"]));

        std::string maxRateStr = pumpObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
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

        std::string minRateStr = shakerObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::Frequency minRate = minRateValue * UtilsJSON::getFrequencyUnits(shakerObj["minRangeUnits"]);

        std::string maxRateStr = shakerObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
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

        std::string minRateStr = stirObj["minRange"];
        double minRateValue = std::atof(minRateStr.c_str());
        units::Frequency minRate = minRateValue * UtilsJSON::getFrequencyUnits(stirObj["minRangeUnits"]);

        std::string maxRateStr = stirObj["maxRange"];
        double maxRateValue = std::atof(maxRateStr.c_str());
        units::Frequency maxRate = maxRateValue * UtilsJSON::getFrequencyUnits(stirObj["maxRangeUnits"]);

        return StirWorkingRange(minRate, maxRate);
    } catch (std::exception & e) {
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parseStirWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}













































