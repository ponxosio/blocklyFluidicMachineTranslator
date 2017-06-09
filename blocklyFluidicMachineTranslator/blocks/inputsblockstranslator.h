#ifndef INPUTSBLOCKSTRANSLATOR_H
#define INPUTSBLOCKSTRANSLATOR_H

#include <stdexcept>
#include <string>

#include <json.hpp>

class InputsBlocksTranslator
{
public:
    virtual ~InputsBlocksTranslator(){}

    std::string processInput(const nlohmann::json & functionObj) throw(std::invalid_argument);
};

#endif // INPUTSBLOCKSTRANSLATOR_H
