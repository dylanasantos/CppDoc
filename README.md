# CppDoc
CppDoc is a function documentation generator similar to systems such as Doxygen or Pydoc for C++ programs. Intended to be a very simple and easy generator. <br><br>

**How to Use CppDoc v0.3.0e** <br>
<u>(live example can also be found in demo.cpp)</u> In order to use cppdoc it requires 2 traits: a CppDoc function indicator (//!CPPDOC), and a multi-lined comment inside the function. CppDoc function indicators can be denoted by //!CPPDOC above function names. Inside the multi lined comment they are 4 potential keyword type: **DEF: ARGS: RET: OTHR:**. <br>

**DEF:** Used for the functions description/definition. <br>
**ARGS:** Used for describing function arguments (must clarify by writing (arg variable name): ) <br>
**RET:** Used for describing the functions return <br>
**OTHR:** Any other information that may be found important to the user <br><br>

Each one of these arguments is optional, but cannot be repeated. **Don't like the keywords? You can change them with CppDocKeyWords::modify_key()** <br>
modify_key() takes 2 arguments: a CPPDOC_KEYS enum value for the key type, and a string to represent your new key. (Note: only 1 colon : is permitted for new keys and it must be a trailing colon)<br><br>

**simple_scan() vs recursive_scan() vs specific_scan()** <br>
Each scan method takes the directory given and will read a set group of files to check for CppDoc functions, but they scan the directories differently. <br><br>

**simple_scan():** reads the local directory called for .cpp files and only the local directory for CppDoc functions. <br>
**recursive_scan():** reads the local directory and its subdirectories for .cpp files with CppDoc functions. <br>
**specific_scan():** (currently unable as of version 0.4.0e) reads a custon cppdoc file which permits or forbids certain directories from being naviagted. <br>
**IMPORTANT:** you can call any scan function with either a file or a directory, if you call a scan function with a file, it automatically uses the directory its located inside of for navigating.<br><br><br>


**debug logging** <br>
By default, debug logging is disabled in CppDoc. Debug logging will print out what the cppdoc_maker program is allocating for certain arguments (what was picked up for def, args, ret, etc..). Debug logging is a static variable, it can be both modified through the constructors, as well as through the class and objects themselves. <br><br>

# IMPORTANT:
Not all function types are currently supported. This is only intended for standard declaration as of version 0.4.0e. This means lambdas and templated are not supported.<br><br>

All html documents are logged and hyperlinked inside of your terminal. If you are having issues access the link, any files made by CppDoc will be added inside of cppdoc/documents. (likely reorganized later) <br><br>

If you do noy like the css formatting, you can always change the css file or the design through cppdoc/documentation/reference.css or be replacing reference.css with a separate reference.css file.
