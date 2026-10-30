/* 
    cppdoc.hpp 
        cppdoc.hpp is the primary file of cppdoc and is used for any interactions relating to the internals of cppdoc comment structure
*/

#pragma once

#include<string>
#include<vector>

#include <filesystem>

class CppDoc {
    public:
        inline const static std::string CPPDOC_VERSION = "0.4.0e";
        inline static bool log_debug = false; //displays debug print statements inside terminal

        enum class SCAN_TYPE {
            SIMPLE,
            RECURSIVE,
            SPECIFY
        };

        enum class CPPDOC_KEYS {
            NONE,
            DEF,
            ARGS,
            RET,
            OTHR,
        };

        CppDoc() = default;
        explicit CppDoc(std::filesystem::path, bool=false); //calls recursive_scan on local directory off file put in
        explicit CppDoc(SCAN_TYPE, std::filesystem::path, bool=false); //calls whatever scan is requested on given file

        void simple_scan(std::filesystem::path); //reads through the local directory simple_scan is called on
        void recursive_scan(std::filesystem::path); //reads through local directory and the subdirectories inside that recursive_scan is called on
        void specify_scan(std::filesystem::path); //[INACTIVE] reads a custom cppdoc file for the instructions on which cpp files can be read

    protected:
        inline static const std::filesystem::path CPPDOC_SOURCE = std::filesystem::path(__FILE__).parent_path();
        inline static std::filesystem::path ACTIVE_FILE;

        inline static std::string def_key = "DEF:";
        inline static std::string arg_key = "ARGS:";
        inline static std::string ret_key = "RET:";
        inline static std::string othr_key = "OTHR:";

    private:
        void format_metadata(const std::vector<std::vector<std::string>>&); //formats data for readability & finalizes documentation
        void parse_file(std::filesystem::path); //used for reading for cppdoc function indicators in a file
};


class CppDocKeyWords : CppDoc {
    public:
        CppDocKeyWords() = delete;

        void simple_scan() = delete;
        void recursive_scan() = delete;
        void specify_scan() = delete;

        static std::string grab_key(CPPDOC_KEYS); //grabs requested CPPDOC Key
        static std::string modify_key(CPPDOC_KEYS, std::string_view); //changes requested CPPDOC key to selected string

        static bool is_key(std::string_view); //checks if string is suitable with any type of cppdoc key
        static CPPDOC_KEYS get_type(std::string_view); //attempts to match cppdoc key type to string, returns CPPDOC_KEYS::NONE if not found
};