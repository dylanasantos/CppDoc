#include "cppdoc_maker.hpp"

#include<fstream>
#include<filesystem>

#include<iostream>
#include<string>
#include<sstream>

#include<algorithm>
#include <optional>

static const std::string ENTER_COMMENT = "/*";
static const std::string EXIT_COMMENT = "*/";

static const std::string DEFAULT_INDENT = "\t\t";
static const std::string DEFAULT_SPACER = "\n\n";

namespace {
    //helper function used for checking if the argument uses an extra keyword (const or volatile)
    bool SPECIAL_ARG(std::string_view keyword) noexcept {
        if(keyword == "const" || keyword == "volatile") return true;

        return false;
    }
}

CppDocHTMLMaker::CppDocHTMLMaker(std::string header) {
    this->handle_header(header);
}

bool CppDocHTMLMaker::name_in_args(std::string_view needle, const function_data& haystack) {
    needle = needle.substr(0, needle.size() - 1); //remove the colon attached to each cppdoc arg title (ex: var_name:)

    for(arg_data arg : haystack.args) {
        if(arg.name == needle) return true;
    }
    return false;
}

std::optional<CppDocHTMLMaker::cppdoc_arg> CppDocHTMLMaker::get_from_cppdoc_args(std::string_view needle, const function_data& haystack) {
    for(auto arg : haystack.cppdoc_args) {
        if(arg.var_name == needle) {
            return arg;
        }
    }
    return {};
}

std::string CppDocHTMLMaker::format_data() {
    std::string lines;

    lines += "\n\t" R"(<div class="file_title">
        <h1>)" + this->file_name.filename().string() + R"(</h1>
    </div>)";

    for(function_data fn : this->file_functions) {
        lines += DEFAULT_SPACER + "\t" R"(<div class="function">
        <div class="function_name">
            <h2>)" + fn.function_name + "</h2>\n" + DEFAULT_INDENT +"</div>" + DEFAULT_SPACER;
        
        lines += DEFAULT_INDENT + R"(<div class="function_description">
            <p>)" + fn.cppdoc_def + "</p>\n" + DEFAULT_INDENT + "</div>" + DEFAULT_SPACER;

        lines += DEFAULT_INDENT + R"(<div class="return">
            <h3>returns ()" + fn.return_type + "):</h3>\n" + R"(
            <div class="return_description">
                <p>)" + fn.cppdoc_ret + "</p>\n" + DEFAULT_INDENT + "\t</div>\n" + 
            DEFAULT_INDENT + "</div>" + DEFAULT_SPACER;
        
        lines += DEFAULT_INDENT + R"(<div class="parameters">
            <h3>parameters:)";
            
        if(fn.args.size() == 0) {
            lines += " (N/A)</h3>" + DEFAULT_SPACER; 
        }
        else {
            lines += "</h3>" + DEFAULT_SPACER;
        }

        for(arg_data arg : fn.args) {
            auto cppdoc_arg = get_from_cppdoc_args(arg.name, fn);

            lines += DEFAULT_INDENT + "\t" R"(<div class="parameter_instance">
                <h4>)" + arg.name + "(";

            if(!arg.special_type.empty()) {
                lines += arg.special_type + " ";
            }
            lines += arg.data_type;

            if(!arg.value_const.empty()) {
                lines += " " + arg.value_const;
            }

            if(!arg.default_value.empty()) {
                lines += " | defaults to: " + arg.default_value;
            }
            lines += ")</h4>\n" + DEFAULT_INDENT;

            if(cppdoc_arg) {
                lines += "\t\t" R"(<p class="parameter_description">)" + cppdoc_arg.value().text + "</p>\n"
                + DEFAULT_INDENT;
            }
            lines += "\t</div>\n";
        }
        lines += DEFAULT_INDENT + "</div>" + DEFAULT_SPACER;

        lines += DEFAULT_INDENT + R"(<div class="other">
            <h3>other:</h3>
            <div class="other_description">
                <p>)" + fn.cppdoc_othr + R"(</p>
            </div>)";
        lines += "\n" + DEFAULT_INDENT + "</div>";
        lines += "\n\t</div>";
    }
    return lines;
}

void CppDocHTMLMaker::new_function() {
    function_data next;
    this->file_functions.push_back(next);
}

void CppDocHTMLMaker::handle_header(std::string header)
{
    function_data &current_function = this->file_functions.back();

    std::stringstream seperator(header);
    std::string keyword;

    int parenthesis_ctr = 0;

    arg_data arg_placeholder;
    bool expecting_default = false;

    while(seperator >> keyword)
    {
        if(expecting_default)
        {
            arg_placeholder.default_value = keyword;
            expecting_default = false;
            continue;
        }

        auto search = FUNCTION_INDICATORS.find(keyword);
        if(search != FUNCTION_INDICATORS.end()) 
        {
            switch(search->second) {
                case SPECIAL_TYPE::OPEN_PAR:
                    parenthesis_ctr++;
                    break;
                
                case SPECIAL_TYPE::CLOSE_PAR:
                    parenthesis_ctr--;
                    
                    if(parenthesis_ctr == 0) {
                        if(!current_function.args.empty())
                        {
                            current_function.args.push_back(arg_placeholder);
                            arg_placeholder.clear();
                        }
                    }
                    break;

                case SPECIAL_TYPE::DEFAULT:
                    expecting_default = true;
                    break;
                
                case SPECIAL_TYPE::NEW_ARG:
                    current_function.args.push_back(arg_placeholder);
                    arg_placeholder.clear();
                    break;
                
                case SPECIAL_TYPE::PRENAME:
                    current_function.pre_name_specials.push_back(keyword);
                    break;
                
                case SPECIAL_TYPE::POSTNAME:
                    if(parenthesis_ctr > 0 && SPECIAL_ARG(keyword)) //used for arguments with const & volatile (ex: const int x)
                    {
                        if(!arg_placeholder.data_type.empty()) { //used for examples such as (const int* const x) or (int* const x)
                            arg_placeholder.value_const = keyword;
                        }
                        else {
                            arg_placeholder.special_type = keyword;
                        }
                        break;
                    }

                    current_function.post_arg_specials.push_back(keyword);
                    break;
                
                case SPECIAL_TYPE::POINTER:
                case SPECIAL_TYPE::REFERENCE:
                    arg_placeholder.data_type += keyword;
                    break;

                default:
                    throw std::invalid_argument("!CPPDOC CppDocHTMLMaker::handle_header() could not resolve location of keyword: " 
                        + keyword + " (unexpected behavior)\n");
            }
            continue;
        }
  
        if(current_function.return_type.empty())
        {
            current_function.return_type = keyword;
            continue;
        }
        else if(current_function.function_name.empty())
        {
            current_function.function_name = keyword;
            continue;
        }
        else if(arg_placeholder.data_type.empty())
        {
            arg_placeholder.data_type = keyword;
        }
        else if(arg_placeholder.name.empty())
        {
            arg_placeholder.name = keyword;
        }
    }
}

void CppDocHTMLMaker::handle_body(std::string body) 
{
    function_data &current_function = this->file_functions.back();

    std::stringstream seperator(body);
    std::string word;

    cppdoc_arg new_cppdoc_arg;
    CPPDOC_KEYS current_description = CPPDOC_KEYS::NONE;

    while(seperator >> word)
    {
        if(word == ENTER_COMMENT || word == EXIT_COMMENT) continue;

        CPPDOC_KEYS type_result = CppDocKeyWords::get_type(word); 
        if(type_result != CPPDOC_KEYS::NONE)
        {
            if(current_description == CPPDOC_KEYS::ARGS) 
            {
                current_function.cppdoc_args.push_back(new_cppdoc_arg);
                new_cppdoc_arg.clear();
            }
            current_description = type_result;
            continue;
        } 

        switch(current_description) {
            case CPPDOC_KEYS::DEF:
                current_function.cppdoc_def += word + " ";
                break;
            
            case CPPDOC_KEYS::ARGS:
                if(name_in_args(word, current_function)) 
                {
                    if(!new_cppdoc_arg.var_name.empty())
                    {
                        current_function.cppdoc_args.push_back(new_cppdoc_arg);
                        new_cppdoc_arg.clear();
                    }
                    new_cppdoc_arg.var_name = word.substr(0, word.size() - 1); //removes colon from description (ex: var_name: text)
                }
                else if(!new_cppdoc_arg.var_name.empty()) {
                    new_cppdoc_arg.text += word + " ";
                }
                break;

            case CPPDOC_KEYS::RET:
                current_function.cppdoc_ret += word + " ";
                break;

            case CPPDOC_KEYS::OTHR:
                current_function.cppdoc_othr += word + " ";
                break;
        }
    }

    //debug checks
    if(this->log_debug) 
    {
        std::cout << "FUNCTION: " << current_function.function_name << "\n";
        std::cout << "CPPDOC COMMENT DEFINITION: " << current_function.cppdoc_def << "\n\n";

        std::cout << "CPPDOC ARUMENTS DEFINITION(S):\n";
        for(auto curr : current_function.cppdoc_args) {
            std::cout << "\tARGUMENT: " << curr.var_name << "\n";
            std::cout << "\tDESCRIPTION: " << curr.text << "\n";
        }

        std::cout << "\nCPPDOC COMMENT RETURN: " << current_function.cppdoc_ret << "\n\n";
        std::cout << "CPPDOC COMMENT OTHER: " << current_function.cppdoc_othr << "\n\n\n";
    }
}

//creates the HTML file documentation for the file the object is referring to
std::filesystem::path CppDocHTMLMaker::develop_document()
{
    std::filesystem::path copy = this->ACTIVE_FILE;
    this->file_name = DOCUMENTATION_DIR / copy.replace_extension("html").filename();

    std::ofstream file(this->file_name);
    if(!file) {
        std::cerr << "!cppdoc CppDocHTMLMaker::develop_document() was unable to create file: " << this->file_name << "\n";
        return this->file_name;
    }

    file << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="reference.css">
    <title>)" << this->file_name.filename() << "</title>"
    << "\n\t<!--DOCUMENT FORMATTED VIA CPPDOC VERSION " << CPPDOC_VERSION << "-->"
    "\n</head>" <<
    "\n<body>" <<
    format_data() <<
    "\n</body>\n</html>";
    
    return this->file_name;
}