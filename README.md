# Commands

### terminal commands utility
## Features
* reading/writing config variables
* writing scripts
* scripts are compiled and then executed
* decompiler (can decompile compiled code)
* defining new functions by assigning anonymous function to variable
* internal types supported are: string, int, float, double
* code completion (functions and variables)
* defining commands is easy, they are automatically type checked before called
* easy to use, templatized

## Will support
* pausing execution
* infix math with custom defined operators
* multiple overload of commands

## Cons
* compiles bit long :S


## defining commands
* statically
COMMAND(int, less, (int a, int b)) {
	return a < b;
}

* dynamicly
Command::AddCommand("unbind", [&](std::string msg) {
	cout << msg << endl;
});
