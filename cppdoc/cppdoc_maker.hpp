/* 
    cppdoc_maker.hpp
        cppdoc_maker is used for the conversion of function metdata into smooth HTML formatting and handles all HTML necessities
        HTML necessities consist of keyword format handling, HTML customization, and documentation structuring
*/

#pragma once

#include "cppdoc.hpp"

#include<string>
#include<utility>
#include<optional>

#include<vector>
#include<unordered_map>

class CppDocHTMLMaker : CppDoc {
    public:
        CppDocHTMLMaker() = default;

        enum class SPECIAL_TYPE {
            PRENAME,
            POSTARG,
            OPEN_PAR,
            CLOSE_PAR,
            NEW_ARG,
            DEFAULT
        };

    private:
        friend class CppDoc;

        CppDocHTMLMaker(std::string);

        void push_function(); //creates new trailing function in vector of function data
        
        void handle_header(std::string); //used for handling the header of a function, and properly allocating all keywords properly
        void handle_body(std::string); //handles the internal cppdoc comment (DEF, ARGS, RET, OTHR)
        
        std::filesystem::path develop_document(); //finalizes document into an HTML format, returns directory to HTML file
        
        void simple_scan() = delete;
        void scan() = delete;

        std::string format_data(); //helper function designed for assisting in HTML configuartion of function metadata

        std::filesystem::path file_name;

        struct arg_data {
            std::string name;
            std::string data_type;
            std::string default_value = "N/A";

            void clear() {
                name.clear();
                data_type.clear();
                default_value = "N/A";
            }
        };

        struct function_data {
            std::string function_name;
            std::string return_type;

            std::vector<arg_data> args;

            //stores pre-name-declaration special keywords
            std::vector<std::string> pre_name_specials;

            //stores post-arguments special keywords
            std::vector<std::string> post_arg_specials;

            std::string cppdoc_def;
            std::vector<std::pair<std::string, std::string>> cppdoc_args; //first->name, second->text
            std::string cppdoc_ret;
            std::string cppdoc_othr;
        };

        std::vector<function_data> file_functions;

        std::optional<std::pair<std::string, std::string>> name_in_cppdoc_args(std::string, function_data);
        bool name_in_args(std::string, function_data);

        inline static const std::filesystem::path DOCUMENTATION_DIR = CPPDOC_SOURCE / "documentation";

            //NOTE: needs to consider other specials still
        inline static const std::unordered_map<std::string, SPECIAL_TYPE> KEYWORD_SPECIALS = {
            {"inline", SPECIAL_TYPE::PRENAME}, {"constexpr", SPECIAL_TYPE::PRENAME}, {"virtual", SPECIAL_TYPE::PRENAME},
            {"explicit", SPECIAL_TYPE::PRENAME}, {"static", SPECIAL_TYPE::PRENAME}, {"extern", SPECIAL_TYPE::PRENAME},
            {"friend", SPECIAL_TYPE::PRENAME}
        };

        inline static const std::unordered_map<std::string, SPECIAL_TYPE> OPERATOR_SPECIALS = {
            {"(", SPECIAL_TYPE::OPEN_PAR}, {")", SPECIAL_TYPE::CLOSE_PAR}, {",", SPECIAL_TYPE::NEW_ARG}, {"=", SPECIAL_TYPE::DEFAULT}
        };
};