#include "inputsblockstranslator.h"

const std::string InputsBlocksTranslator::MATHBLOCK_NUMBER_STR = "math_number";
const std::string InputsBlocksTranslator::MATH_NUMBER_LIST_STR = "number_list";
const std::string InputsBlocksTranslator::STRING_STR = "text";
const std::string InputsBlocksTranslator::STRING_LIST_STR = "text_list";

using json = nlohmann::json;

std::string InputsBlocksTranslator::processInput(const nlohmann::json & inputObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"block_type"}, inputObj);

        std::string input;
        std::string type = inputObj["block_type"];
        if (type.compare(MATHBLOCK_NUMBER_STR) == 0) {
            input = processMathNumber(inputObj);
        } else if (type.compare(MATH_NUMBER_LIST_STR) == 0) {
            input = processMathNumberList(inputObj);
        } else if (type.compare(STRING_STR) == 0) {
            input = processString(inputObj);
        } else if (type.compare(STRING_LIST_STR) == 0) {
            input = processStringList(inputObj);
        } else {
            throw(std::invalid_argument("unknow input type: " + type));
        }
        return input;
    } catch (std::exception & e) {
        throw(std::invalid_argument("InputsBlocksTranslator::processInput. Exception ocurred " + std::string(e.what())));
    }
}

std::string InputsBlocksTranslator::processMathNumber(const nlohmann::json & inputObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"value"}, inputObj);

        return inputObj["value"];
    } catch (std::exception & e) {
        throw(std::invalid_argument("InputsBlocksTranslator::processMathNumber. Exception ocurred " + std::string(e.what())));
    }
}

std::string InputsBlocksTranslator::processMathNumberList(const nlohmann::json & inputObj) throw(std::invalid_argument) {
    try {
        std::stringstream numberList;
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"containerList"}, inputObj);

        json containerList = inputObj["containerList"];

        auto it = containerList.begin();
        numberList << processInput(*it);
        ++it;

        for(; it != containerList.end(); ++it) {
            numberList << "," << processInput(*it);
        }
        return numberList.str();
    } catch (std::exception & e) {
        throw(std::invalid_argument("InputsBlocksTranslator::processMathNumberList. Exception ocurred " + std::string(e.what())));
    }
}

std::string InputsBlocksTranslator::processString(const nlohmann::json & inputObj) throw(std::invalid_argument) {
    try {
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"TEXT"}, inputObj);
        return inputObj["TEXT"];
    } catch (std::exception & e) {
        throw(std::invalid_argument("InputsBlocksTranslator::processString. Exception ocurred " + std::string(e.what())));
    }
}

std::string InputsBlocksTranslator::processStringList(const nlohmann::json & inputObj) throw(std::invalid_argument) {
    try {
        std::stringstream textList;
        UtilsJSON::checkPropertiesExists(std::vector<std::string>{"containerList"}, inputObj);

        json containerList = inputObj["containerList"];

        auto it = containerList.begin();
        textList << processInput(*it);
        ++it;

        for(; it != containerList.end(); ++it) {
            textList << "," << processInput(*it);
        }
        return textList.str();
    } catch (std::exception & e) {
        throw(std::invalid_argument("InputsBlocksTranslator::processStringList. Exception ocurred " + std::string(e.what())));
    }
}

