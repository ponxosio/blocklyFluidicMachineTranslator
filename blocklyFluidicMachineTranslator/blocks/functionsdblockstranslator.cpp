#include "functionsdblockstranslator.h"

using json = nlohmann::json;

std::shared_ptr<ValvePluginRouteFunction> FunctionsdBlocksTranslator::processValveFunction(
        const nlohmann::json & functionObj,
        ValveNode::TruthTable & truthTable)
    throw(std::invalid_argument)
{
    try {
        PluginConfiguration configObj = fillConfigurationObj(functionObj);

        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"truthTable"}, functionObj);
        truthTable = parseTruthTable(functionObj["truthTable"]);

        return std::make_shared<ValvePluginRouteFunction>(NULL, configObj);
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

        return std::make_shared<PumpPluginFunction>(NULL, configObj, wRange);
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

PumpWorkingRange FunctionsdBlocksTranslator::parsePumpWorkingRange(const nlohmann::json & pumpObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "minRange",
                                             "minRangeVolumeUnits",
                                             "minRangeTimeUnits",
                                         "maxRange",
                                         "maxRangeVolumeUnits",
                                         "maxRangeTimeUnits"}, pluginObj);
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
        throw(std::invalid_argument("FunctionsdBlocksTranslator::parsePumpWorkingRange. Exception ocurred " + std::string(e.what())));
    }
}













































