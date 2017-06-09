#include "blocklyfluidicmachinetranslator.h"

using json = nlohmann::json;

const std::string BlocklyFluidicMachineTranslator::OPEN_CONTAINER_STR = "OPEN_CONTAINER";
const std::string BlocklyFluidicMachineTranslator::CLOSE_CONTAINER_STR = "CLOSE_CONTAINER";
const std::string BlocklyFluidicMachineTranslator::PUMP_STR = "PUMP";
const std::string BlocklyFluidicMachineTranslator::VALVE_STR = "VALVE";

BlocklyFluidicMachineTranslator::BlocklyFluidicMachineTranslator(const std::string & path, std::shared_ptr<CommandSender> communications) :
    path(path)
{
    factory = std::make_shared<PythonPluginAbstractFactory>(communications);
}

BlocklyFluidicMachineTranslator::~BlocklyFluidicMachineTranslator() {

}

std::shared_ptr<FluidicMachineModel> BlocklyFluidicMachineTranslator::translateFile() {
    std::ifstream in(filePath);
    json js;
    try {
        in >> js;
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "default_rate",
                                             "default_rate_volume_units",
                                             "default_rate_time_units",
                                             "integer_precission",
                                             "decimal_precission",
                                             "connections"}, js);

        model = std::make_shared<MachineGraph>();

        json connections = js["connections"];
        for(auto it = connections.begin(); it != connections.end(); ++it) {
            json configurationBlock = *it;
            processConfigurationBlock(configurationBlock);
        }
        processConnectionMap();

        std::string defaultRateStr = js["default_rate"];
        double defaultRate = std::atof(defaultRateStr.c_str());
        units::Volumetric_Flow defaultRateUnits = UtilsJSON::getVolumeUnits(js["default_rate_volume_units"]) /
                                                  UtilsJSON::getTimeUnits(js["default_rate_time_units"]);

        std::string integerPrecisionStr = js["integer_precission"];
        int integerPrecission = std::atoi(integerPrecisionStr.c_str());

        std::string decimalPrecissionStr = js["decimal_precission"];
        int decimalPrecission = std::atoi(decimalPrecissionStr.c_str());

        std::shared_ptr<PrologTranslationStack> pTranslationStack = std::make_shared<PrologTranslationStack>();

        std::shared_ptr<FluidicMachineModel> createdModel =
                std::make_shared<FluidicMachineModel>(model,
                                                      pTranslationStack,
                                                      integerPrecission,
                                                      decimalPrecission,
                                                      defaultRate,
                                                      defaultRateUnits);
        createdModel->updatePluginFactory(factory);
        return createdModel;
    } catch (std::exception & e) {
        throw(std::invalid_argument("BlocklyFluidicMachineTranslator::translateFile. Exception ocurred " + std::string(e.what())));
    }
}

void BlocklyFluidicMachineTranslator::processConfigurationBlock(const nlohmann::json & blockObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{
                                             "reference",
                                             "type",
                                             "functions",
                                             "number_pins"}, js);

        std::string id = blockObj["reference"];

        std::string pinNumberStr = blockObj["number_pins"];
        int numberPins = std::atoi(pinNumberStr.c_str());

        std::string nodeType = blockObj["type"];
        if (nodeType.compare(OPEN_CONTAINER_STR) == 0) {
            UtilsJSON::checkPropertiesExists(std::vector<std::string>{"extra_functions"}, js);

            processContainer(id, ContainerNode::open, numberPins, blockObj["functions"], blockObj["extra_functions"]);
        } else if (nodeType.compare(CLOSE_CONTAINER_STR) == 0) {
            UtilsJSON::checkPropertiesExists(std::vector<std::string>{"extra_functions"}, js);

            processContainer(id, ContainerNode::close, numberPins, blockObj["functions"], blockObj["extra_functions"]);
        } else if (nodeType.compare(PUMP_STR) == 0) {
            processPump(id, numberPins, blockObj["functions"]);
        } else if (nodeType.compare(PUMP_STR) == 0) {
            processValve(id, numberPins, blockObj["functions"]);
        } else {
            throw(std::invalid_argument("unknow node type: " + nodeType));
        }

        for(int i = 1; i <= numberPins; i++) {
            std::string portName = "port" + std::to_string(i);
            if(UtilsJSON::hasProperty(portName, blockObj)) {
                int target = processReferenceBlock(blockObj[portName]);
                addNewConnection(id, i-1, target);
            } else {
                throw(std::invalid_argument("missing port: " + portName));
            }
        }
    } catch (std::exception & e) {
        throw(std::invalid_argument("BlocklyFluidicMachineTranslator::processConfigurationBlock. Exception ocurred " + std::string(e.what())));
    }
}

void BlocklyFluidicMachineTranslator::processPump(const std::string & id, int pinNumber, const nlohmann::json & functionsObj) {
    bool reversible;
    std::shared_ptr<PumpPluginFunction> pump = FunctionsdBlocksTranslator::processPumpFunction(functionsObj, reversible);

    std::shared_ptr<PumpNode> pumpPtr = std::make_shared<PumpNode>(getReferenceId(id),
                                                                   pinNumber,
                                                                   reversible ? PumpNode::bidirectional : PumpNode::unidirectional,
                                                                   pump);
    model->addNode(pumpPtr);
}

void BlocklyFluidicMachineTranslator::processValve(const std::string & id, int pinNumber, const nlohmann::json & functionsObj) {
    ValveNode::TruthTable tTable;
    std::shared_ptr<ValvePluginRouteFunction> valve = FunctionsdBlocksTranslator::processValveFunction(functionsObj, tTable);

    std::shared_ptr<ValveNode> valvePtr = std::make_shared<ValveNode>(getReferenceId(id),
                                                                     pinNumber,
                                                                     tTable,
                                                                     valve);
    model->addNode(valvePtr);
}

void BlocklyFluidicMachineTranslator::processContainer(
        const std::string & id,
        ContainerNode::ContainerType type,
        int pinNumber,
        const nlohmann::json & functionsObj,
        const nlohmann::json & extraFunctionsObj)
{
    units::Volume minVolume;
    units::Volume capacity;
    FunctionsdBlocksTranslator::processGlasswareFunction(functionsObj, minVolume, capacity);

    std::shared_ptr<ContainerNode> nodePtr = std::make_shared<ContainerNode>(getReferenceId(id), pinNumber, type, capacity);

    std::vector<std::shared_ptr<Function>> functions = FunctionsdBlocksTranslator::processFunctions(extraFunctionsObj);
    for(auto func : functions) {
        nodePtr->addOperation(func);
    }
    model->addNode(nodePtr);
}

void BlocklyFluidicMachineTranslator::processConnectionMap() {

}

int BlocklyFluidicMachineTranslator::getReferenceId(const std::string & reference) {
    auto finded = variableIdMap.find(reference);
    if (finded != variableIdMap.end()) {
        return finded->second;
    } else {
        int nextAvailableId = serie.getNextValue();
        variableIdMap.insert(std::make_pair(reference, nextAvailableId));
        return nextAvailableId;
    }
}




















