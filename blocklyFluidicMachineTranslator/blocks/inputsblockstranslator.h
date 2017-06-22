#ifndef INPUTSBLOCKSTRANSLATOR_H
#define INPUTSBLOCKSTRANSLATOR_H

#include <stdexcept>
#include <string>
#include <sstream>

#include <json.hpp>

#include <utils/utilsjson.h>

class InputsBlocksTranslator
{
    static const std::string MATHBLOCK_NUMBER_STR;
    static const std::string MATH_NUMBER_LIST_STR;
    static const std::string STRING_STR;
    static const std::string STRING_LIST_STR;

public:
    virtual ~InputsBlocksTranslator(){}

    static std::string processInput(const nlohmann::json & inputObj) throw(std::invalid_argument);

protected:

    static std::string processMathNumber(const nlohmann::json & inputObj) throw(std::invalid_argument);
    static std::string processMathNumberList(const nlohmann::json & inputObj) throw(std::invalid_argument);

    static std::string processString(const nlohmann::json & inputObj) throw(std::invalid_argument);
    static std::string processStringList(const nlohmann::json & inputObj) throw(std::invalid_argument);
};

#endif // INPUTSBLOCKSTRANSLATOR_H
