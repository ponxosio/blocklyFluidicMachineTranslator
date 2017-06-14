#ifndef BLOCKLYFLUIDICMACHINETRANSLATOR_H
#define BLOCKLYFLUIDICMACHINETRANSLATOR_H

#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <json.hpp>

#include <constraintengine/prologtranslationstack.h>

#include <commonmodel/functions/function.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>

#include <fluidicmachinemodel/fluidicmachinemodel.h>
#include <fluidicmachinemodel/machinegraph.h>
#include <fluidicmachinemodel/fluidicnode/containernode.h>
#include <fluidicmachinemodel/fluidicnode/pumpnode.h>
#include <fluidicmachinemodel/fluidicnode/valvenode.h>

#include <utils/AutoEnumerate.h>
#include <utils/utilsjson.h>

#include "blocklyFluidicMachineTranslator/blocks/functionsdblockstranslator.h"
#include "blocklyfluidicmachinetranslator_global.h"

class BLOCKLYFLUIDICMACHINETRANSLATORSHARED_EXPORT BlocklyFluidicMachineTranslator
{
    static const std::string OPEN_CONTAINER_STR;
    static const std::string CLOSE_CONTAINER_STR;
    static const std::string PUMP_STR;
    static const std::string VALVE_STR;

public:
    BlocklyFluidicMachineTranslator(const std::string & path, std::shared_ptr<PluginAbstractFactory> factory);
    virtual ~BlocklyFluidicMachineTranslator();

    std::shared_ptr<FluidicMachineModel> translateFile();

protected:
    std::string path;

    std::shared_ptr<MachineGraph> model;
    std::shared_ptr<PluginAbstractFactory> factory;

    AutoEnumerate serie;
    std::unordered_map<std::string, int> variableIdMap;
    std::unordered_map<int,std::unordered_map<int,int>> connectionsMap;

    void processConfigurationBlock(const nlohmann::json & blockObj) throw(std::invalid_argument);

    void processPump(const std::string & id, int pinNumber, const nlohmann::json & functionsObj);
    void processValve(const std::string & id, int pinNumber, const nlohmann::json & functionsObj);
    void processContainer(const std::string & id,
                          ContainerNode::ContainerType type,
                          int pinNumber,
                          const nlohmann::json & functionsObj,
                          const nlohmann::json & extraFunctionsObj);

    int processReferenceBlock(const nlohmann::json & referenceObj) throw(std::invalid_argument);

    void processConnectionMap();
    void addNewConnection(int source, int sourcePort, int target);

    int getReferenceId(const std::string & reference);
};

#endif // BLOCKLYFLUIDICMACHINETRANSLATOR_H
