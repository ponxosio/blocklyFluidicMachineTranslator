#include "blocklyfluidicmachinetranslator.h"

using json = nlohmann::json;

const std::string BlocklyFluidicMachineTranslator::OPEN_CONTAINER_STR = "OPEN_CONTAINER";
const std::string BlocklyFluidicMachineTranslator::CLOSE_CONTAINER_STR = "CLOSE_CONTAINER";
const std::string BlocklyFluidicMachineTranslator::PUMP_STR = "PUMP";
const std::string BlocklyFluidicMachineTranslator::VALVE_STR = "VALVE";

BlocklyFluidicMachineTranslator::BlocklyFluidicMachineTranslator(const std::string & path, std::shared_ptr<PluginAbstractFactory> factory) :
    path(path)
{
    this->factory = factory;
}

BlocklyFluidicMachineTranslator::~BlocklyFluidicMachineTranslator() {

}

BlocklyFluidicMachineTranslator::ModelMappingTuple BlocklyFluidicMachineTranslator::translateFile() {
    std::ifstream in(path);
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
        processTwins();

        double defaultRate = js["default_rate"];
        units::Volumetric_Flow defaultRateUnits = UtilsJSON::getVolumeUnits(js["default_rate_volume_units"]) /
                                                  UtilsJSON::getTimeUnits(js["default_rate_time_units"]);

        int integerPrecission = js["integer_precission"];
        int decimalPrecission = js["decimal_precission"];

        std::shared_ptr<PrologTranslationStack> pTranslationStack = std::make_shared<PrologTranslationStack>();

        std::shared_ptr<FluidicMachineModel> createdModel =
                std::make_shared<FluidicMachineModel>(model,
                                                      pTranslationStack,
                                                      integerPrecission,
                                                      decimalPrecission,
                                                      defaultRate,
                                                      defaultRateUnits);
        createdModel->updatePluginFactory(factory);

        std::shared_ptr<FluidicModelMapping> mapping = std::make_shared<FluidicModelMapping>(createdModel);

        return std::make_tuple(createdModel, mapping);
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
                                             "number_pins"}, blockObj);

        std::string id = blockObj["reference"];

        int numberPins = blockObj["number_pins"];;

        std::string nodeType = blockObj["type"];
        if (nodeType.compare(OPEN_CONTAINER_STR) == 0) {
            UtilsJSON::checkPropertiesExists(std::vector<std::string>{"extra_functions"}, blockObj);
            processOpenContainer(id, numberPins, blockObj["functions"], blockObj["extra_functions"]);
        } else if (nodeType.compare(CLOSE_CONTAINER_STR) == 0) {
            UtilsJSON::checkPropertiesExists(std::vector<std::string>{"extra_functions"}, blockObj);

            processCloseContainer(id, numberPins, blockObj["functions"], blockObj["extra_functions"]);
        } else if (nodeType.compare(PUMP_STR) == 0) {
            processPump(id, numberPins, blockObj["functions"]);
        } else if (nodeType.compare(VALVE_STR) == 0) {
            processValve(id, numberPins, blockObj["functions"]);
            processValveTwins(id, blockObj);
        } else {
            throw(std::invalid_argument("unknow node type: " + nodeType));
        }

        processDirectionsPorts(id, blockObj);

        for(int i = 1; i <= numberPins; i++) {
            std::string portName = "port" + std::to_string(i);
            if(UtilsJSON::hasProperty(portName, blockObj)) {
                float target = processReferenceBlock(blockObj[portName]);
                addNewConnection(getReferenceId(id), i-1, target);
            } else {
                throw(std::invalid_argument("missing port: " + portName));
            }
        }
    } catch (std::exception & e) {
        throw(std::invalid_argument("BlocklyFluidicMachineTranslator::processConfigurationBlock. Exception ocurred " + std::string(e.what())));
    }
}

void BlocklyFluidicMachineTranslator::processDirectionsPorts(const std::string & id, const nlohmann::json & blockObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists({"in_ports", "out_ports"}, blockObj);

        std::unordered_set<int> inPorts;
        json inPortsList = blockObj["in_ports"];
        for(auto it = inPortsList.begin(); it != inPortsList.end(); ++it) {
            int actualInPort = *it;
            inPorts.insert(actualInPort-1);
        }

        std::unordered_set<int> outPorts;
        json outPortsList = blockObj["out_ports"];
        for(auto it = outPortsList.begin(); it != outPortsList.end(); ++it) {
            int actualInPort = *it;
            outPorts.insert(actualInPort-1);
        }

        addDirectionPorts(getReferenceId(id), inPorts, outPorts);
    } catch (std::exception & e) {
        throw(std::invalid_argument("BlocklyFluidicMachineTranslator::processDirectionsPorts(), exception ocurred: " + std::string(e.what())));
    }
}

void BlocklyFluidicMachineTranslator::processPump(const std::string & id, int pinNumber, const nlohmann::json & functionsObj) {
    bool reversible;
    std::shared_ptr<PumpPluginFunction> pump = FunctionsdBlocksTranslator::processPumpFunction(functionsObj, reversible);

    int idNum = getReferenceId(id);
    std::shared_ptr<PumpNode> pumpPtr = std::make_shared<PumpNode>(idNum,
                                                                   pinNumber,
                                                                   reversible ? PumpNode::bidirectional : PumpNode::unidirectional,
                                                                   pump);
    model->addNode(pumpPtr);
}

void BlocklyFluidicMachineTranslator::processValve(const std::string & id, int pinNumber, const nlohmann::json & functionsObj) {
    ValveNode::TruthTable tTable;
    std::shared_ptr<ValvePluginRouteFunction> valve = FunctionsdBlocksTranslator::processValveFunction(functionsObj, tTable);

    int referenceId = getReferenceId(id);
    std::shared_ptr<ValveNode> valvePtr = std::make_shared<ValveNode>(referenceId,
                                                                      pinNumber,
                                                                      tTable,
                                                                      valve);
    model->addNode(valvePtr);
}

void BlocklyFluidicMachineTranslator::processValveTwins(const std::string & id, const nlohmann::json & functionsObj) {
    if (UtilsJSON::hasProperty("number_twins", functionsObj)) {
        std::unordered_set<int> twins = {getReferenceId(id)};

        int numberTwins = functionsObj["number_twins"];
        for(int i=0; i < numberTwins; i++) {

            std::string name = "twin" + std::to_string(i + 1);
            if (UtilsJSON::hasProperty(name, functionsObj)) {
                int twinId = std::floorf(processReferenceBlock(functionsObj[name]));
                twins.insert(twinId);
            }
        }
        twinsVector.push_back(twins);
    }
}

void BlocklyFluidicMachineTranslator::processOpenContainer(
        const std::string & id,
        int pinNumber,
        const nlohmann::json & functionsObj,
        const nlohmann::json & extraFunctionsObj)
{
    units::Volume minVolume;
    units::Volume capacity;
    FunctionsdBlocksTranslator::processOpenGlasswareFunction(functionsObj, minVolume, capacity);

    std::shared_ptr<ContainerNode> nodePtr =
            std::make_shared<ContainerNode>(getReferenceId(id), pinNumber, ContainerNode::open, capacity);

    if (extraFunctionsObj != nullptr) {
        std::vector<std::shared_ptr<Function>> functions = FunctionsdBlocksTranslator::processFunctions(extraFunctionsObj);
        for(auto func : functions) {
            nodePtr->addOperation(func);
        }
    }
    model->addNode(nodePtr);
}

void BlocklyFluidicMachineTranslator::processCloseContainer(
        const std::string & id,
        int pinNumber,
        const nlohmann::json & functionsObj,
        const nlohmann::json & extraFunctionsObj)
{
    units::Volume minVolume;
    units::Volume capacity;

    FunctionsdBlocksTranslator::processCloseGlasswareFunction(functionsObj, minVolume, capacity);

    int idNum = getReferenceId(id);
    std::shared_ptr<ContainerNode> nodePtr =
            std::make_shared<ContainerNode>(idNum, pinNumber, ContainerNode::close, capacity);

    if (extraFunctionsObj != nullptr) {
        std::vector<std::shared_ptr<Function>> functions = FunctionsdBlocksTranslator::processFunctions(extraFunctionsObj);
        for(auto func : functions) {
            nodePtr->addOperation(func);
        }
    }
    model->addNode(nodePtr);
}

float BlocklyFluidicMachineTranslator::processReferenceBlock(const nlohmann::json & referenceObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"reference"}, referenceObj);

        if (referenceObj["block_type"] == "part_copy") {
            return 0.1 + processReferenceBlock(referenceObj["reference"]);
        } else {
            std::string reference = referenceObj["reference"];
            return getReferenceId(reference);
        }
    } catch (std::exception & e) {
        throw(std::invalid_argument("BlocklyFluidicMachineTranslator::processReferenceBlock. exception: " + std::string(e.what())));
    }
}


void BlocklyFluidicMachineTranslator::processConnectionMap() throw(std::invalid_argument) {
    std::unordered_set<int> proccessed;

    //first process the nodes with fixed directions
    for(const auto & directionPair: directedConnectionsMapsIn) {
        int source = directionPair.first;
        const std::unordered_set<int> & inPorts = directionPair.second;
        const std::unordered_set<int> & outPorts = directedConnectionsMapsOut[source];

        const std::unordered_map<float,int> & portsConnections = connectionsMap[source];
        for(const auto & connectPair : portsConnections) {
            int target = std::floorf(connectPair.first);
            float prime = connectPair.first - target;

            int sourcePort = connectPair.second;
            int targetPort = connectionsMap[target][source + prime];

            if(inPorts.find(sourcePort) != inPorts.end()) {
                if (proccessed.find(target) == proccessed.end()) {
                    model->connectNodes(target, source, targetPort, sourcePort);
                }
            } else if (outPorts.find(sourcePort) != outPorts.end()) {
                if (proccessed.find(target) == proccessed.end()) {
                    model->connectNodes(source, target, sourcePort, targetPort);
                }
            } else {
                throw(std::invalid_argument("BlocklyFluidicMachineTranslator::processConnectionMap. node's " + std::to_string(source) +
                                            "port " + std::to_string(sourcePort) + " is not an in/out port"));
            }
        }
        connectionsMap.erase(source); //once this node is processed remove it from the map so is not processed again in the next for
        proccessed.insert(source); //add to processed so edges are not duplicated
    }

    //then process valves
    const std::unordered_set<int> & valves = model->getValvesIdsSet();
    for(int source : valves) {
        const std::unordered_map<float,int> & portsConnections = connectionsMap[source];

        for(const auto & portConnection: portsConnections) {
            int target = std::floorf(portConnection.first);
            float prime = portConnection.first - target;

            int sourcePort = portConnection.second;
            int targetPort = connectionsMap[target][source + prime];

            if (proccessed.find(target) == proccessed.end()) {
                model->connectNodes(source, target, sourcePort, targetPort);
            }
        }
        connectionsMap.erase(source); //once this node is processed remove it from the map so is not processed again in the next for
        proccessed.insert(source); //add to processed so edges are not duplicated
    }

    //then process the reamining nodes that does not have fixed directions
    for(const auto & connectionPair: connectionsMap) {
        int source = connectionPair.first;
        const std::unordered_map<float,int> & portsConnections = connectionPair.second;

        for(const auto & portConnection: portsConnections) {
            int target = std::floorf(portConnection.first);
            float prime = portConnection.first - target;

            int sourcePort = portConnection.second;
            int targetPort = connectionsMap[target][source + prime];

            if (proccessed.find(target) == proccessed.end()) {
                model->connectNodes(source, target, sourcePort, targetPort);
            }
        }
        proccessed.insert(source); //add to processed so edges are not duplicated
    }
}

void BlocklyFluidicMachineTranslator::processTwins() {
    for(const std::unordered_set<int> & twins: twinsVector) {
        model->setValvesAsTwins(twins);
    }
}

void BlocklyFluidicMachineTranslator::addNewConnection(int source, int sourcePort, float target) {
    auto finded = connectionsMap.find(source);
    if (finded != connectionsMap.end()) {
        std::unordered_map<float,int> & portMap = finded->second;
        portMap.insert(std::make_pair(target, sourcePort));
    } else {
        std::unordered_map<float,int> portMap;
        portMap.insert(std::make_pair(target, sourcePort));
        connectionsMap.insert(std::make_pair(source, portMap));
    }
}

void BlocklyFluidicMachineTranslator::addDirectionPorts(
        int id,
        const std::unordered_set<int> & inPorts,
        const std::unordered_set<int> & outPorts)
    throw(std::invalid_argument)
{
    if (directedConnectionsMapsIn.find(id) == directedConnectionsMapsIn.end() &&
        directedConnectionsMapsOut.find(id) == directedConnectionsMapsOut.end())
    {
        directedConnectionsMapsIn.insert(std::make_pair(id, inPorts));
        directedConnectionsMapsOut.insert(std::make_pair(id, outPorts));
    } else {
        throw(std::invalid_argument("BlocklyFluidicMachineTranslator::addDirectionPorts, " + std::to_string(id) +
                                    " already has connection values."));
    }
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




















