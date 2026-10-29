# CppDoc
CppDoc is a function documentation generator similar to systems such as Doxygen or Pydoc for C++ programs. Intended to be a very simple and easy generator. <br><br>

**How to Use CppDoc v0.3.0e** <br>
[(live example can also be found in demo.cpp)] In order to use cppdoc it requires 2 traits: a CppDoc function indicator, and a multi-lined comment inside the function. CppDoc function indicators can be denoted by //!CPPDOC above function names. Inside the multi lined comment they are 4 potential keyword type: **DEF: ARGS: RET: OTHR:**. <br>

**DEF:** Used for the functions description/definition. <br>
**ARGS:** Used for describing function arguments (must clarify by writing (arg variable name): ) <br>
**RET:** Used for describing the functions return <br>
**OTHR:** Any other information that may be found important to the user <br>

Each one of these arguments is optional, but cannot be repeated. **Don't like the keywords? You can change them with CppDocKeyWords::modify_key()** <br>
modify_key() takes 2 arguments: a CPPDOC_KEYS enum value for the key type, and a string to represent your new key. <br><br>

**simple_scan() vs scan()** <br>
simple_scan() takes the directory that you call CppDoc in and searches through all .cpp files for potential CppDoc functions. **simple_scan() does not work on nested .cpp files**. To instruct CppDoc to search through other file sets you must use scan() (currently unable in version 0.3.0e). <br><br>

**debug logging** <br>
By default, debug logging is disabled in CppDoc. Debug logging will print out what the cppdoc_maker program is allocating for certain arguments (what was picked up for def, args, ret, etc..). Debug logging is a static variable, it can be both modified through the constructors, as well as through the class and objects themselves. <br><br>

# IMPORTANT:
Not all function types are currently supported. This is only intended for standard declaration as of version 0.3.0e <br><br>

All html documents are logged and hyperlinked inside of your terminal. If you are having issues access the link, any files made by CppDoc will be added inside of cppdoc/documents. (likely reorganized later) <br><br>

If you do noy like the CSS formatting, you can always change the css file or the design through cppdoc/documentation/reference.css.
