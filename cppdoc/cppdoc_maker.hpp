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
            POSTNAME,
            OPEN_PAR,
            CLOSE_PAR,
            NEW_ARG,
            DEFAULT,
            REFERENCE,
            POINTER
        };

    private:
        friend class CppDoc;

        CppDocHTMLMaker(std::string); //takes function header for variable & type formatting

        void new_function(); //creates new trailing function in vector of function data for next function
        
        void handle_header(std::string); //used for handling the header of a function, and properly allocating all keywords properly
        void handle_body(std::string); //handles the internal cppdoc comment (DEF, ARGS, RET, OTHR)
        
        std::filesystem::path develop_document(); //finalizes document into an HTML format, returns directory to HTML file
        
        void simple_scan() = delete;
        void recursive_scan() = delete;
        void specify_scan() = delete;

        std::string format_data(); //helper function designed for assisting in HTML configuartion of function metadata

        std::filesystem::path file_name;

        struct arg_data {
            std::string name;
            std::string data_type;
            
            std::string default_value = "N/A"; 
            std::string special_type;
            std::string value_const;

            void clear() {
                name.clear();
                data_type.clear();

                default_value = "N/A";
                special_type.clear();
                value_const.clear();
            }

            bool empty() {
                return name.empty() && data_type.empty();
            }
        };

        struct cppdoc_arg {
            std::string var_name;
            std::string text;

            void clear() {
                var_name.clear();
                text.clear();
            }
        };

        struct function_data {
            std::string function_name;
            std::string return_type;

            std::vector<arg_data> args;

            std::vector<std::string> pre_name_specials; //stores pre-name-declaration special keywords
            std::vector<std::string> post_arg_specials; //stores post-arguments special keywords

            std::string cppdoc_def;
            std::vector<cppdoc_arg> cppdoc_args;
            std::string cppdoc_ret;
            std::string cppdoc_othr;
        };

        std::vector<function_data> file_functions;

        std::optional<cppdoc_arg> get_from_cppdoc_args(std::string_view, const function_data&);
        bool name_in_args(std::string_view, const function_data&);

        inline static const std::filesystem::path DOCUMENTATION_DIR = CPPDOC_SOURCE / "documentation";

            //NOTE: needs to consider other specials still
        inline static const std::unordered_map<std::string, SPECIAL_TYPE> FUNCTION_INDICATORS = {
            {"(", SPECIAL_TYPE::OPEN_PAR}, {")", SPECIAL_TYPE::CLOSE_PAR}, {",", SPECIAL_TYPE::NEW_ARG}, 
            {"=", SPECIAL_TYPE::DEFAULT}, {"*", SPECIAL_TYPE::POINTER}, {"&", SPECIAL_TYPE::REFERENCE},
            {"inline", SPECIAL_TYPE::PRENAME}, {"constexpr", SPECIAL_TYPE::PRENAME}, {"virtual", SPECIAL_TYPE::PRENAME}, 
            {"explicit", SPECIAL_TYPE::PRENAME}, {"static", SPECIAL_TYPE::PRENAME}, {"friend", SPECIAL_TYPE::PRENAME}, 
            {"const", SPECIAL_TYPE::POSTNAME}, {"volatile", SPECIAL_TYPE::POSTNAME}
        };
};