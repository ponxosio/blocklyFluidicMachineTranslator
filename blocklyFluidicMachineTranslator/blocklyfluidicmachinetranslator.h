#ifndef BLOCKLYFLUIDICMACHINETRANSLATOR_H
#define BLOCKLYFLUIDICMACHINETRANSLATOR_H

#include <fstream>
#include <memory>
#include <stdexcept>
#include <tuple>
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

#include <fluidicmodelmapping/fluidicmodelmapping.h>

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

    typedef std::tuple<std::shared_ptr<FluidicMachineModel>, std::shared_ptr<FluidicModelMapping>> ModelMappingTuple;

    BlocklyFluidicMachineTranslator(const std::string & path, std::shared_ptr<PluginAbstractFactory> factory);
    virtual ~BlocklyFluidicMachineTranslator();

    ModelMappingTuple translateFile();

    const std::unordered_map<std::string, int> & getVariableIdMap() const {
        return variableIdMap;
    }
protected:
    std::string path;

    std::shared_ptr<MachineGraph> model;
    std::shared_ptr<PluginAbstractFactory> factory;

    AutoEnumerate serie;
    std::unordered_map<std::string, int> variableIdMap;

    std::unordered_map<int,std::unordered_map<float,int>> connectionsMap;
    std::unordered_map<int,std::unordered_set<int>> directedConnectionsMapsIn;
    std::unordered_map<int,std::unordered_set<int>> directedConnectionsMapsOut;

    std::vector<std::unordered_set<int>> twinsVector;

    void processConfigurationBlock(const nlohmann::json & blockObj) throw(std::invalid_argument);
    void processDirectionsPorts(const std::string & id, const nlohmann::json & blockObj) throw(std::invalid_argument);

    void processPump(const std::string & id, int pinNumber, const nlohmann::json & functionsObj);
    void processValve(const std::string & id, int pinNumber, const nlohmann::json & functionsObj);
    void processValveTwins(const std::string & id, const nlohmann::json & functionsObj);

    void processOpenContainer(const std::string & id,
                              int pinNumber,
                              const nlohmann::json & functionsObj,
                              const nlohmann::json & extraFunctionsObj);
    void processCloseContainer(const std::string & id,
                              int pinNumber,
                              const nlohmann::json & functionsObj,
                              const nlohmann::json & extraFunctionsObj);

    float processReferenceBlock(const nlohmann::json & referenceObj) throw(std::invalid_argument);

    void processConnectionMap() throw(std::invalid_argument);
    void processTwins();

    void addNewConnection(int source, int sourcePort, float target);
    void addDirectionPorts(int id, const std::unordered_set<int> & inPorts, const std::unordered_set<int> & outPorts) throw(std::invalid_argument);

    int getReferenceId(const std::string & reference);
};

#endif // BLOCKLYFLUIDICMACHINETRANSLATOR_H
