#include<iostream>
#include "cppdoc.hpp"

struct obj {
    int x;
};

//!CPPDOC    
int example_function(int param1, double param2, obj test={3})
{   
    /*
        DEF: Definition/explanation of the function

        ARGS: (optional)
            param1: param1 definition (optional)
            param2: something man idk

        RET: definition/explanation of functions return (optional)
            
        OTHR: program will display return type, function name, definition, any default values, and any parameter definitons
        this text will be written underneath an "other" section
    */

    return 1;
}

//!CPPDOC    
int example_function2(int       param1, 
    double param2=10) {
    /*
        DEF: Definition/explanation of the function

        ARGS: (optional)
            param1: param1 definition (optional)

        RET: definition/explanation of functions return (optional)
            
        OTHR: Hello!
    */

    if(true) {
        //random edgecase
    }

    return 1; 
}

//other text not apart of a function SHOULD NOT BE DISPLAYED
constexpr bool DEBUG_STATUS = true;

int main()
{
    std::cout << "Test\n";

    CppDoc my_formatter(__FILE__, DEBUG_STATUS);
}