#include "cppdoc.hpp"
#include "cppdoc_maker.hpp"

#include<fstream>
#include<filesystem>

#include<iostream>
#include<algorithm>
#include<regex>
#include <vector>

static const std::string WS_EDGECASES = " \t\n\r\f\v";
static const std::string CPPDOC_INDICATOR = "//!CPPDOC";

static const std::string ENTER_COMMENT = "/*";
static const std::string EXIT_COMMENT = "*/";

namespace {
    const std::string HYPERLINK_START = "\033]8;;";
    const std::string ASCII_ESC = "\033\\";
    const std::string HYPERLINK_END = "\033]8;;\033\\";

    bool is_cppdoc_indicator(std::string& line) {
        if(line.empty()) return false;

        auto start = line.find_first_not_of(WS_EDGECASES);
        auto end = line.find_last_not_of(WS_EDGECASES);

        if(start == std::string::npos) return false;
        if(end == std::string::npos || end < start) end = line.size();

        std::string text = line.substr(start, (end - start) + 1);
        return text == CPPDOC_INDICATOR;
    }

    void collapse_line(std::string &line, bool inside_function) 
    {   
        line.erase(line.begin(), find_if(line.begin(), line.end(), [](char ch){
            return !std::isspace(ch);
        }));

        if(line.empty()) return;

        auto new_end = std::unique(line.begin(), line.end(), [](char left, char right) {
            return std::isspace(left) && std::isspace(right);
        });
        line.erase(new_end, line.end());

        auto ending_index = line.find_last_not_of(WS_EDGECASES);
        if(ending_index != std::string::npos) line.resize(ending_index + 1);

        try 
        {
            if(!inside_function)
            {
                line = std::regex_replace(line, std::regex(R"(\s*\(\s*)"), " ( ");
                line = std::regex_replace(line, std::regex(R"(\s*\)\s*)"), " ) ");
                line = std::regex_replace(line, std::regex(R"(\s*=\s*)"), " = ");
                line = std::regex_replace(line, std::regex(R"(\s*,\s*)"), " , ");
                line = std::regex_replace(line, std::regex(R"(\s*\*\s*)"), " * ");
            }
            else {
                line = std::regex_replace(line, std::regex(R"(\s*:\s*)"), ": ");
            }
            
            //clean up any double spaces created by the replacement
            auto clean_end = std::unique(line.begin(), line.end(), [](unsigned char left, unsigned char right) {
                return std::isspace(left) && std::isspace(right);
            });
            line.erase(clean_end, line.end());
        } 
        catch (const std::regex_error &re) 
        {
            std::cerr << "WARNING: !CPPDOC collapse_line encountered regex_error; " << re.what() << "\n";
        }
    }


    int count_bracket(std::string_view bracket_type, std::string_view line) 
    {
        char open = bracket_type.at(0);
        char close = bracket_type.at(1);

        int bracket_ctr = 0;

        bracket_ctr += std::count(line.begin(), line.end(), open);
        bracket_ctr -= std::count(line.begin(), line.end(), close);

        return bracket_ctr;
    }
}



CppDoc::CppDoc(std::filesystem::path dir, bool debug_status) {
    this->log_debug = debug_status;
    this->recursive_scan(dir);
}



CppDoc::CppDoc(SCAN_TYPE type, std::filesystem::path dir, bool debug_status) {
    this->log_debug = debug_status;

    switch(type)
    {
        case SCAN_TYPE::SIMPLE: 
            this->simple_scan(dir);
            break;

        case SCAN_TYPE::RECURSIVE: 
            this->recursive_scan(dir);
            break;

        case SCAN_TYPE::SPECIFY:
            this->specify_scan(dir);
            break;

        default:
            throw std::invalid_argument("CppDoc() invalid SCAN_TYPE type presented");
    }
}



void CppDoc::simple_scan(std::filesystem::path local_dir)
{
    try
    {
        std::filesystem::path active_dir = std::filesystem::is_directory(local_dir) ? local_dir : local_dir.parent_path();

        if(std::filesystem::exists(active_dir) && std::filesystem::is_directory(active_dir))
        {
            for(const auto& item : std::filesystem::directory_iterator(active_dir))
            {
                std::filesystem::path item_path = item.path();

                if(item_path.extension() == ".cpp")
                {
                    this->ACTIVE_FILE = item_path;
                    parse_file(item_path);
                }
            }
        }
        else
        {
            std::cerr << "!cppdoc -> an error occured trying to acces the file directory you provided, please make sure you provided the correct path" 
                << " (path: " << local_dir << ")\n";
        }
    }
    catch(const std::filesystem::filesystem_error &fse)
    {
        std::cerr << "!cppdoc -> an error occured with parsing through your directory: [" << fse.what() << "]\n";
    }
}



void CppDoc::recursive_scan(std::filesystem::path local_dir)
{
    try
    {
        std::filesystem::path active_dir = std::filesystem::is_directory(local_dir) ? local_dir : local_dir.parent_path();

        if(std::filesystem::exists(active_dir) && std::filesystem::is_directory(active_dir))
        {
            for(const auto& item : std::filesystem::recursive_directory_iterator(active_dir))
            {
                std::filesystem::path item_path = item.path();

                if(item_path.extension() == ".cpp") 
                {
                    this->ACTIVE_FILE = item_path;
                    parse_file(item_path);
                }
            }
        }
    }
    catch(const std::filesystem::filesystem_error &fse)
    {
        std::cerr << "!cppdoc -> an error occured with parsing through your directory: [" << fse.what() << "]\n";
    }
}



void CppDoc::specify_scan(std::filesystem::path cppdoc_instructions){}



void CppDoc::format_metadata(const std::vector<std::vector<std::string>>& data)
{
    /* TODO:
        IMPORTANT: make sure special keyword types can be enabled/disabled, there should also be a whitelist & blacklist option
    */
    CppDocHTMLMaker manager;
    for(std::vector function : data)
    {
        manager.new_function();

        std::string header_full = "";
        size_t parenthesis_ctr = 0;
        size_t header_end = 0;

        for(std::string line : function) //looks for range of parameter header (in case args extends > 1 line)
        {
            parenthesis_ctr += count_bracket("()", line);

            if(parenthesis_ctr == 0)
            {
                header_full += line;
                break;
            }

            header_full += line;
            header_end++;
        }
        manager.handle_header(header_full);
        
        std::string cppdoc_body = "";
        bool in_comment = false;

        for(auto it = function.begin() + header_end; it != function.end(); it++)
        {
            std::string line = *it;

            if(line.find(ENTER_COMMENT) != std::string::npos) in_comment = true;

            if(line.find(EXIT_COMMENT) != std::string::npos)
            {
                cppdoc_body += line;
                in_comment = false;
            }
            if(in_comment) cppdoc_body += line + " ";
        }
        manager.handle_body(cppdoc_body);
    }
    auto file = manager.develop_document();
    auto full_file_path = std::filesystem::absolute(file);

    std::cout << "!CPPDOC: " << file.filename() << " document generated at: " << 
        HYPERLINK_START << full_file_path.string() << ASCII_ESC << full_file_path << HYPERLINK_END << "\n";
}


void CppDoc::parse_file(std::filesystem::path file_dir)
{
    std::ifstream file(file_dir);
    std::string line;

    std::vector<std::vector<std::string>> function_metadata;
    std::vector<std::string> text_placeholder;
    int bracket_ctr = 0; //accounts for internal brackets {} to avoid exiting a function early
    int parenth_ctr = 0; //accounts for internal parenthesis () in a functions args to assure the correct starting bracket 

    bool contains_cppdoc = false;
    bool is_cppdoc_function = false;
    bool in_args = false;
    bool inside_cppdoc_function = false;
    
    bool in_string = false;
    bool valid_open_bracket = true;
    bool valid_close_bracket = true;

    size_t string_start = std::string::npos;
    size_t string_end = std::string::npos;

    while(std::getline(file, line))
    {
        if(is_cppdoc_indicator(line))
        {
            if(!contains_cppdoc) contains_cppdoc = true;

            is_cppdoc_function = true;
            continue;
        }

        if(is_cppdoc_function)
        {
            collapse_line(line, inside_cppdoc_function);

            size_t string_pos = line.find("\"");
            if(string_pos != std::string::npos) 
            {
                if(!in_string)
                {
                    string_start = string_pos;
                    in_string = true;
                }
                else
                {
                    string_end = string_pos;
                    in_string = false;
                }
            }

            if(line.find("(") != std::string::npos && !in_args && !inside_cppdoc_function) 
            {
                in_args = true;
                parenth_ctr += count_bracket("()", line);
            }

            size_t arg_end = line.find(")");
            if(in_args && arg_end != std::string::npos) {
                parenth_ctr += count_bracket("()", line);

                if(parenth_ctr != 0)
                {
                    arg_end = std::string::npos;
                }
            }

            size_t open_bracket_pos = line.find("{");
            size_t close_bracket_pos = line.find("}");

            if(in_string) //handles cases where brackets may be inside strings
            {
                if(open_bracket_pos >= string_start && open_bracket_pos <= string_end) {
                    valid_open_bracket = false;
                } else {
                    valid_open_bracket = true;
                }

                if(close_bracket_pos >= string_start && close_bracket_pos <= string_end) {
                    valid_close_bracket = false;
                } else {
                    valid_close_bracket = true;
                }
            }

            if(arg_end != std::string::npos) //handles case where brackets may be inside args
            {
                if(open_bracket_pos <= arg_end) {
                    valid_open_bracket = false;
                } else {
                    valid_open_bracket = true;
                }

                if(close_bracket_pos <= arg_end) {
                    valid_close_bracket = false;
                } else {
                    valid_close_bracket = true;
                }
            }

            if(!inside_cppdoc_function && open_bracket_pos != std::string::npos)
            {
                inside_cppdoc_function = true;
                bracket_ctr++;
            }
            else if(inside_cppdoc_function)
            {
                if(open_bracket_pos != std::string::npos && valid_open_bracket) bracket_ctr++;
                if(close_bracket_pos != std::string::npos && valid_close_bracket) bracket_ctr--;

                if(bracket_ctr == 0)
                {
                    text_placeholder.push_back(line);
                    function_metadata.push_back(text_placeholder);
                    text_placeholder.clear();

                    inside_cppdoc_function = false;
                    is_cppdoc_function = false;
                    continue;
                }
            }
            text_placeholder.push_back(line);
        }
    }
    if(contains_cppdoc) format_metadata(function_metadata);
}



std::string CppDocKeyWords::grab_key(CPPDOC_KEYS key)
{
    using key_type = CppDocKeyWords;

    switch(key)
    {
        case CPPDOC_KEYS::DEF: return key_type::def_key;

        case CPPDOC_KEYS::ARGS: return key_type::arg_key;

        case CPPDOC_KEYS::RET: return key_type::ret_key;

        case CPPDOC_KEYS::OTHR: return key_type::othr_key;

        default:
            throw std::invalid_argument("CppDocKeyWords grab_key() called with invalid key type (make sure to use CPPDOC_KEYS enum)");
    }
}



std::string CppDocKeyWords::modify_key(CPPDOC_KEYS key, std::string_view new_key)
{
    if(new_key == def_key || new_key == arg_key || new_key == ret_key || new_key == othr_key)
        throw std::invalid_argument("CppDocKeyWords modify_key() cannot have two keywords share the same key");

    if(std::count(new_key.begin(), new_key.end(), ':') > 1)
        throw std::invalid_argument("CppDocKeyWords modify_key() new_key does not permit keys with > 1 colon");

    if(new_key.find(":") < new_key.size() - 1) 
        throw std::invalid_argument("CppDocKeyWords modify_key() new_key only permits trailing colons");
    
    std::string temp;

    if(new_key.empty()) 
        throw std::invalid_argument("CppDocKeyWords modify_key() cannot replace a keyword with an empty string");

    switch(key)
    {
        case CPPDOC_KEYS::DEF:
            temp = def_key;
            def_key = new_key;
            return temp;

        case CPPDOC_KEYS::ARGS:
            temp = arg_key;
            arg_key = new_key;
            return temp;

        case CPPDOC_KEYS::RET:
            temp = ret_key;
            ret_key = new_key;
            return temp;

        case CPPDOC_KEYS::OTHR:
            temp = othr_key;
            othr_key = new_key;
            return temp;

        default:
            throw std::invalid_argument("CppDocKeyWords modify_key() invalid key type");
    }
    throw std::invalid_argument("CppDocKeyWords modify_key() ? received unexpected behavior");
}



bool CppDocKeyWords::is_key(std::string_view potential_key)
{
    if(potential_key == def_key || potential_key == arg_key || potential_key == ret_key || potential_key == othr_key) return true;
    return false;
}



CppDoc::CPPDOC_KEYS CppDocKeyWords::get_type(std::string_view potential_key)
{
    if(potential_key == def_key) return CPPDOC_KEYS::DEF;
    if(potential_key == arg_key) return CPPDOC_KEYS::ARGS;
    if(potential_key == ret_key) return CPPDOC_KEYS::RET;
    if(potential_key == othr_key) return CPPDOC_KEYS::OTHR;

    return CPPDOC_KEYS::NONE;
}