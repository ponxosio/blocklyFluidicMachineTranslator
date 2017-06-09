#ifndef FUNCTIONSDBLOCKSTRANSLATOR_H
#define FUNCTIONSDBLOCKSTRANSLATOR_H

#include <memory>
#include <stdexcept>
#include <vector>

#include <json.hpp>

#include <commonmodel/functions/function.h>
#include <commonmodel/functions/pumppluginfunction.h>
#include <commonmodel/functions/valvepluginroutefunction.h>

#include <commonmodel/functions/ranges/pumpworkingrange.h>

#include <fluidicmachinemodel/fluidicnode/valvenode.h>

#include <utils/utils.h>
#include <utils/utilsjson.h>

#include "blocklyFluidicMachineTranslator/blocks/inputsblockstranslator.h"

class FunctionsdBlocksTranslator
{
public:
    virtual ~FunctionsdBlocksTranslator(){}

    static std::vector<std::shared_ptr<Function>> processFunctions(const nlohmann::json & functionObj) throw(std::invalid_argument);

    static std::shared_ptr<ValvePluginRouteFunction> processValveFunction(const nlohmann::json & functionObj,
                                                                          ValveNode::TruthTable & truthTable) throw(std::invalid_argument);

    static std::shared_ptr<PumpPluginFunction> processPumpFunction(const nlohmann::json & functionObj,
                                                                   bool & reversible) throw(std::invalid_argument);

    static void processGlasswareFunction(const nlohmann::json & functionObj,
                                         units::Volume & minVolume,
                                         units::Volume & maxVolume) throw(std::invalid_argument);

protected:
    static PluginConfiguration fillConfigurationObj(const nlohmann::json & pluginObj) throw(std::invalid_argument);

    static ValveNode::TruthTable parseTruthTable(const nlohmann::json & truthTableObj) throw(std::invalid_argument);
    static std::vector<std::unordered_set<int>> parseConnectedPins(const nlohmann::json & connectedPins);

    static PumpWorkingRange parsePumpWorkingRange(const nlohmann::json & pumpObj) throw(std::invalid_argument);
};

#endif // FUNCTIONSDBLOCKSTRANSLATOR_H
