#include "commands.hpp"
#include <cstring>
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>
#include <sstream>
#include <cmath>
namespace Commands {
	
// #define VARIABLE_CHARACTER_SET_SIZE 2*26+10
#define VARIABLE_CHARACTER_SET_SIZE 256
struct node {
	int cmd;
	std::array<node*, VARIABLE_CHARACTER_SET_SIZE> nodes;
	node() {
		cmd = -1;
		for(node*& n : nodes) {
			n = nullptr;
		}
	}
	~node() {
		for(node*& n : nodes) {
			if(n) delete n;
			n = 0;
		}
	}
};
}
Command Command::singleton __attribute__((init_priority(200))) ();

using namespace Commands;

// -------------- [ ARG ] ---------------------
inline void try_convert(Arg& a, Arg::type_enum t) {
	if(a.type == t) return;
	
	switch(t) {
		case Arg::t_string:
			a.s = (std::string)a;
			break;
		case Arg::t_int:
			a.i = (int)a;
			break;
		case Arg::t_float:
			if(a.type == Arg::t_string) {
				try {
					a.f = std::stof(a.s);
				}catch(...) {
					
				}
			} else if(a.type == Arg::t_int) {
				a.f = a.i;
			} else if(a.type == Arg::t_double) {
				a.f = a.d;
			}
			break;
		case Arg::t_double:
			if(a.type == Arg::t_string) {
				try {
					a.d = std::stod(a.s);
				}catch(...) {
					
				}
			} else if(a.type == Arg::t_int) {
				a.d = a.i;
			} else if(a.type == Arg::t_double) {
				a.d = a.d;
			}
			break;
	}
}

int Arg::to_int() {
	return *this;
}

float Arg::to_float() {
	if(type == t_int)
		return i;
	else if(type == t_string) {
		try {
			return std::stof(s);
		} catch(...) {
			return 0;
		}
	} else if(type == t_float) {
		return f;
	} else if(type == t_double) {
		return (float)d;
	}
}
double Arg::to_double() {
	if(type == t_int)
		return i;
	else if(type == t_string) {
		try {
			return std::stod(s);
		} catch(...) {
			return 0;
		}
	} else if(type == t_float) {
		return (int)f;
	} else if(type == t_double) {
		return (int)d;
	}
}
std::string Arg::to_string() {
	return *this;
}

/*
Arg::operator double() {
	if(type == t_int)
		return i;
	else if(type == t_string) {
		try {
			return std::stod(s);
		} catch(...) {
			return 0;
		}
	} else if(type == t_float) {
		return (int)f;
	} else if(type == t_double) {
		return (int)d;
	}
}
Arg::operator float() {
	if(type == t_int)
		return i;
	else if(type == t_string) {
		try {
			return std::stof(s);
		} catch(...) {
			return 0;
		}
	} else if(type == t_float) {
		return f;
	} else if(type == t_double) {
		return (float)d;
	}
}
*/

Arg& Arg::operator=(const Arg& a) {
	// if(this == 0) return *this;
	switch(a.type) {
		case t_float:
			f = a.f;
			break;
		case t_double:
			d = a.d;
			break;
		case t_string:
			new (&s) std::string(a.s);
			break;
		default:
			i = a.i;
	}
	type = a.type;
	
	return *this;
}
Arg::operator int() {
	if(type == t_int)
		return i;
	else if(type == t_string) {
		try {
			return std::stoi(s);
		} catch(...) {
			return 0;
		}
	} else if(type == t_float) {
		return (int)f;
	} else if(type == t_double) {
		return (int)d;
	}
}
Arg::operator std::string() {
	if(type == t_string) {
		return s;
	} else if(type == t_int) {
		return std::to_string(i);
	} else if(type == t_float) {
		return std::to_string(f);
	} else if(type == t_double) {
		return std::to_string(d);
	} else return "";
}

// -----------------------------------------------------------
// --------------------------[ CONFIG ]-----------------------
// -----------------------------------------------------------

#define EVAL_START '('
#define EVAL_END ')'

#define TEMPLATE_START '['
#define TEMPLATE_END ']'

#define FUNCTION_DELIMITER ';'
#define STRING_OPERATOR '\"'

#define GET_VARIABLE_FUNCTION "get"
#define SET_VARIABLE_FUNCTION "set"
#define IF_FUNCTION "if"
#define GOTO_FUNCTION "#"

#define VARIABLE_MARKER '$'
#define ESCAPING_CHARACTER '\\'

#define ADD_BASIC_MATH_FUNCTIONS
#define ADD_ADVANCED_MATH_FUNCTIONS
#define ADD_TRIGONOMETRIC_FUNCTIONS
#define ADD_CONVERSION_FUNCTIONS

// $variable = 5
// if disabled: variable = 5 won't work
// #define MUST_BE_VARIABLE_FOR_EQUAL_SET

// ----------------[ CONSTRUCTOR ]-------------------------

Command::Command() : m_root_functions(new node()), m_root_variables(new node()) {

#ifdef ADD_BASIC_MATH_FUNCTIONS
	add_command("+", [](int a, std::vector<Arg> arg) {
		int sum=a;
		for(auto& b : arg) {
			sum += b.to_int(); 
		}
		return sum;
	});
	add_command("+f", [](float a, std::vector<Arg> arg) {
		float sum=0;
		for(auto& a : arg) {
			sum += a.to_float();
		}
		return sum;
	});
	add_command("-", [](std::vector<Arg> arg) {
		int sum=arg[0];
		for(int i=1; i < arg.size(); i++) {
			Arg& a = arg[i];
			sum -= a.to_int();
		}
		return sum;
	});
	add_command("-f", [](std::vector<Arg> arg) {
		float sum=arg[0];
		for(int i=1; i < arg.size(); i++) {
			Arg& a = arg[i];
			try_convert(a, Arg::t_float);
			sum -= a.to_float();
		}
		return sum;
	});
	add_command("*", [](int first, std::vector<Arg> arg) {
		int prod=first;
		for(auto& a : arg) {
			prod *= a.to_int(); 
		}
		return prod;
	});
	add_command("*f", [](std::vector<Arg> arg) {
		float prod=1;
		for(auto& a : arg) {
			try_convert(a, Arg::t_float);
			prod *= a.to_float(); 
		}
		return prod;
	});
	add_command("/", [](int a, int b) {
		if(b == 0) return 999999;
		return a/b;
	});
	add_command("/f", [](float a, float b) -> float{
		if(b == 0) return 999999;
		return a/b;
	});
	add_command("^", [](std::vector<Arg> arg) {
		int res = 0;
		for(auto& a : arg) {
			res ^= a; 
		}
		return res;
	});

	add_command("<", [](int a, int b) {
		return a < b;
	});
	add_command(">", [](int a, int b) {
		return a > b;
	});
	add_command(">=", [](int a, int b) {
		return a >= b;
	});
	add_command("<=", [](int a, int b) {
		return a <= b;
	});
	add_command("", [](int a, int b) {
		return a <= b;
	});
#endif	

#ifdef ADD_ADVANCED_MATH_FUNCTIONS
add_command("pow", [](Arg a, Arg b) -> int{
	try_convert(a, Arg::t_int);
	try_convert(b, Arg::t_int);
	return pow(a,b);
});
#endif
	
#ifdef ADD_TRIGONOMETRIC_FUNCTIONS
	add_command("sin", [](float a) {
		return sin(a);
	});
	add_command("cos", [](float a) {
		return cos(a);
	});
#endif
	
#ifdef ADD_CONVERSION_FUNCTIONS
	add_command("float", [](Arg a) -> float {
		try_convert(a, Arg::t_float);
		return a.f;
	});
	add_command("double", [](Arg a) -> double {
		try_convert(a, Arg::t_double);
		return a.d;
	});
	add_command("string", [](Arg a) -> std::string {
		return (std::string)a;
	});
	add_command("int", [](Arg a) -> int {
		return a;
	});
#endif
}
// ---------------------------------------------------------------------------------
// ------------------------------------[ DEBUG ]------------------------------------
// ---------------------------------------------------------------------------------

#define debug(x)
#define debug2(x)

/*
{ t_void, t_int, t_float, t_double, t_string, t_string_ref, // basic types
		   t_end, t_eval, t_template, t_executable, t_variable, 
		   t_function, t_get, t_set, t_if, t_param, t_loop  }
 */
char const* type_to_name[50] = { "t_void", "t_int", "t_float", "t_double", "t_string", "t_string_ref", 
	"t_eval", "t_template", "t_executable", "t_variable", "t_function", "t_get", "t_set", "t_if", "t_param", "t_loop", "t_goto" };
void Arg::dump() const {
	// if(type >= 19 || type < 0) return;
	cout << "(" << type_to_name[(int)type] << ", ";
	switch(type) {
		case t_float:
			cout << f; break;
		case t_double:
			cout << d; break;
		case t_string:
			cout << '"' << s << '"'; break;
		default:
			cout << i; break;
	}
	cout << ")";
}

void Command::printCompiledCode(const std::vector<Arg>& code) {
	cout << code.size() << " code: ";
	for(const auto& a : code) {
		a.dump();
		cout << ", ";
	}
	cout << endl;
}


// ------------------------------------------------------------------------
// --------------------------------[ DECOMPILER ]--------------------------
// ------------------------------------------------------------------------

std::string Command::get_executable_text(const Arg& arg) {
	if(arg.type == Arg::t_executable) {
		auto it = m_executables.find(arg.i);
		if(it != m_executables.end()) {
			std::stringstream s;
			debug2(printCompiledCode(it->second.code);)
			decompile_code(s, it->second.code, 0, it->second.code.size());
			return s.str();
		} else {
			debug2(cout << "executable not found\n");
		}
	} else {
		debug2(cout << "not executable\n");
	}
	return "";
}

void Command::decompile_code(std::stringstream& s, std::vector<Arg>& e, int ofs, int len) {
	bool first = true;
	len += ofs;
	bool is_ident = false;
	debug2(cout << "decompiling" << endl;)
	for(int i=ofs; i < len; i++) {
		Arg& a = e[i];
		switch(a.type) {
			case Arg::t_template:
				debug2(cout << "template: ";
				a.dump();
				cout << endl;)
				
				s << " " << TEMPLATE_START;
				i++;
				decompile_code(s, e, i, a.i);
				i+=a.i-1;
				s << " " << TEMPLATE_END;
				break;
			case Arg::t_eval:
				debug2(cout << "evaluating: ";
				a.dump();
				cout << endl;)
				
				s << " " << EVAL_START;
				i++;
				decompile_code(s, e, i, a.i);
				i+=a.i-1;
				s << EVAL_END;
				break;
			case Arg::t_param:
				if(!first) {
					s << " ";
				}
				if(a.i > 0) {
					s << VARIABLE_MARKER << a.i;
				} else if(a.i == 0) {
					s << "$0";
				} else if(a.i < 0) {
					int indx = -(a.i+1);
					s << VARIABLE_MARKER;
					if(indx > 0)
						s << indx;
					s << "...";
				} else
					s << VARIABLE_MARKER << a.i;
				
				break;
			case Arg::t_loop:
				if(!first) {
					s << " ";
				}
				s << VARIABLE_MARKER;
				break;
			case Arg::t_if:
			case Arg::t_get:
			case Arg::t_set:
			case Arg::t_goto:
			case Arg::t_function: {
				if(!first) {
					s << FUNCTION_DELIMITER;
				}
				if(a.type == Arg::t_if) {
					s << " " IF_FUNCTION;
				} else if(a.type == Arg::t_get) {
					s << " " GET_VARIABLE_FUNCTION;
				} else if(a.type == Arg::t_goto) {
					s << " " GOTO_FUNCTION;
				} else if(a.type == Arg::t_set) {
					s << " " SET_VARIABLE_FUNCTION;
					is_ident = true;
				} else if(a.type == Arg::t_function) {
					debug2(cout << "function" << endl;)
					Arg &b = e[i+1];
					if(b.type == Arg::t_int) {
						if(!first) {
							s << " ";
						}
						s << m_strings_reverse[b.i];
						i++;
					} else if(e[i+1].type == Arg::t_string) {
						if(!first) {
							s << " ";
						}
						s << e[i+1].s;
						i++;
					}
					
				}
				first = false;
				break;
			}
			case Arg::t_variable: {
				s << " " << VARIABLE_MARKER << m_strings_reverse[a.i];
				break;
			}
			case Arg::t_int:
				if(is_ident) {
					s << " " << VARIABLE_MARKER << m_strings_reverse[a.i];
					is_ident = false;
				} else
					s << " " << a.i;
				break;
			case Arg::t_float:
				s << a.f;
				s << "f ";
				break;
			case Arg::t_string:
				if(is_ident) {
					s << " " << a.s;
					is_ident = false;
				} else
					s << " " << STRING_OPERATOR << a.s << STRING_OPERATOR;
				break;
			case Arg::t_double:
				s << " " << a.d;
				break;
		}
	}
}

// ---------------------------------------------------------------------------------------
// ------------------------[ CODE COMPLETION AND INTROSPECTION ]--------------------------
// ---------------------------------------------------------------------------------------


char ident_to_index(char c) {
	if(c == '/' || c == ' ' || c == '$')
		return -1;
	return c;
	/*
	char l = 'z'-'a';
	if(c >= 'a' && c <= 'z')
		c = c - 'a';
	else if(c >= 'A' && c <= 'Z') 
		c = c - 'A' + l;
	else if(c >= '0' && c <= '9')
		c = c - '0' + 2*l;
	else
		return -1;
	return c;
	*/
}
char index_to_ident(char c) {
	return c;
	/*
	char l = 'z'-'a';
	if(c <= l)
		c = c+'a';
	else if(c <= 2*l)
		c = c-l + 'A';
	else if(c <= 2*l+10) 
		c = c-2*l+'0';
	return c;
	*/
}

void Command::add_to_tree(node* n, std::string s) {
	for(auto& cc : s) {
		
		char c = ident_to_index(cc);
		if(c == -1) return;
		
		if(!n->nodes[c]) {
			n->nodes[c] = new node();
		}
		
		n = n->nodes[c];
	}
	n->cmd = alloc_string(s);
}

void Command::fill(node* n, std::vector<std::string>& vec, std::string s, int limit) {
	if(!n || vec.size() >= limit) return;
	if(n->cmd != -1) {
		vec.push_back(s);
	}
	char ch = 0;
	for(node* c : n->nodes) {
		if(c) fill(c, vec, s+index_to_ident(ch), limit);
		ch++;
	}
}

std::vector<std::string> Command::search(const std::string& cmd, int cursor, int limit) {
	node* n = sweep_node(cmd, cursor);
	std::vector<std::string> vec;
	fill(n, vec, "", limit);
	return vec;
}

node* Command::sweep_node(const std::string& cmd, int cursor) {
	int i;
	node* n = nullptr;
	for(i=cursor; i > 0; i--) {
		if(cmd[i] == '[' || cmd[i] == '(' || cmd[i] == ';' || cmd[i] == ' ' || cmd[i] == '/') break;
		if(cmd[i] == '$') {
			n = m_root_variables;
			break;
		}
	}
	if(!n) n = m_root_functions;
	
	if(i < 0) i = 0;
	int start=i+1, stop=cmd.size();
	for(; i < cmd.size(); i++) {
		if(ident_to_index(cmd[i]) != -1) {
			start = i;
			break;
		}
	}
	for(; i < cmd.size(); i++) {
		if(ident_to_index(cmd[i]) == -1) {
			stop = i;
			break;
		}
	}
	if(start == -1 || stop == -1) return nullptr;
	std::string ident = cmd.substr(start, stop-start);
	
	
	for(i=0; i < ident.size(); i++) {
		char c = ident_to_index(ident[i]);
		if(c == -1) return nullptr;
		
		if(n) {
			n = n->nodes[c];
		} else
			return nullptr;
	}
	return n;
}

std::string Command::complete(const std::string& cmd, int cursor) {
	node* n = sweep_node(cmd, cursor);
	std::string additional = "";
	while(n) {
		if(n->cmd != -1) return additional;
		node* d = nullptr;
		char ch = 0;
		char lc = 0;
		for(node* c : n->nodes) {
			if(c) {
				if(!d) {
					d = c;
					lc = index_to_ident(ch);
				} else {
					return additional;
				}
			}
			
			ch++;
		}
		additional += lc;
		n = d;
	}
	
	return additional;
	
}

const std::string Command::help(const std::string& command) {
	auto it = m_strings.find(command);
	if(it != m_strings.end()) {		
		const std::string& sign = m_command_signatures[it->second];
		std::string ret = command + " ";
		for(auto& s : sign) {
			switch(s) {
				case 'f': ret += "float "; break;
				case 'd': ret += "double "; break;
				case 'i': ret += "int "; break;
				case 'v': ret += "... "; break;
				case 's': ret += "string "; break;
			}
		}
		return ret;
	} else {
		return "command not found";
	}
}

//	-------------------------------------------------------------------------------------------
//	-------------------------------[ PARSER (COMPILER) ]---------------------------------------
//	-------------------------------------------------------------------------------------------

static bool isNumeric(std::string& s) {
	if(s.size() == 0) return false;
	if(!isdigit(s[0]) && s[0] != '.' && s[0] != '-') return false;
	int num_digits = 0;
	for(int i=1; i < s.size()-1; i++) {
		if(!isdigit(s[i]) && s[i] != '.') return false;
		num_digits = 0;
	}
	
	if(!(s.back() == 'f' && num_digits != 0) && !isdigit(s.back())) return false;
	return true;
}
static bool isOperator(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '&' || c == '<' || c == '>' || c == '#';
}
static bool isIdent(char c) {
	return isalnum(c) || isOperator(c) || c == '_' || c == '.' || c == '?';
}

Commands::Arg Command::parseVariable(std::vector<Arg>& code, const char*& b) {
	char const* s = b;
	if(*s != VARIABLE_MARKER) return 0;
	Arg a;
	s++;
	char const* start = s;
	if(isIdent(*s)) {
		for(; *s; s++) {
			if(!isIdent(*s)) break;
		}
		std::string ident(start, (int)(s-start));
		auto it = m_strings.find(ident);
		if(it != m_strings.end()) {
			a = Arg(it->second, Arg::t_variable);
		} else {
			int alloc = alloc_variable(ident);
			a = Arg(alloc, Arg::t_variable);
		}
		
		for(; *s && *s != FUNCTION_DELIMITER && *s != EVAL_END && *s != TEMPLATE_END && *s != '='; s++);
		
	} else {
		for(; *s; s++) {
			if(!isIdent(*s)) break;
		}
		std::string ident(start, (int)(s-start));
		
		int num_count = 0;
		for(int i=0; i < ident.size(); i++) {
			if(!isdigit(s[i])) break;
			num_count++;
		}
		if(num_count > 0)
			a = Arg(std::stoi(ident), Arg::t_param);
		if(std::string(s+num_count, 3) == "...") {
			if(num_count == 0)
				a = Arg(-1, Arg::t_param);
			else
				a.i = -a.i;
		}
	}
	
	b = s;
	return a;
}

const char* Command::parseCode(std::vector<Arg>& code, const char* s, std::string& error_log) {
	bool isescaping = false;
	bool iscommand = true;
	bool isvariable = false;
	bool isnewline = code.empty();
	bool variable_function = false;
	int last_func = code.size();
	int argument = 0;
	enum { space, ident, string } state = space;
	
	#define UPDATE_ARGS \
		if(last_func < code.size()) { \
			debug(cout << "last_func: " << last_func << " args: " << argument << endl); \
			if(code[last_func].type == Arg::t_loop && argument != 0) { \
				error_log = "$ (loop operator) must not take arguments"; \
				return 0; \
			} \
			variable_function = false; \
			code[last_func].i = argument; \
		}
		
	const char * start = s;
	Arg a;
	while(s) {
		const char &c = *s;
		
		if(state != string) {
			if(isnewline && (c == ';' || c == '#')) {
				while(*s && *s != '\n')s++;
				continue;
			}
			if( // is identifier or escape character?
				(iscommand && c != '$' && c != ' ' && c != TEMPLATE_START && c != TEMPLATE_END && c != EVAL_START && c != EVAL_END) || 
				isIdent(c) || 
				c == ESCAPING_CHARACTER) {
				
				if(state == space) {
					start = s;
					state = ident;
				}
				if(c == ESCAPING_CHARACTER) {
					debug(cout << "escaping\n");
					isescaping = true;
					s++;
					start++;
					continue;
				}
				
				
			} else if(!isescaping) {
				 if(c == VARIABLE_MARKER) {
					if(iscommand)
						variable_function = true;
					state = ident;
					if(isvariable && start == s) {
						// reference to current executable
						code.emplace_back(Arg::t_current_executable_reference);
					}
					s++;
					start = s;
					isvariable = true;
					continue;
				} else if(c == EVAL_START) {
					if(iscommand) {
						iscommand = false;
						last_func = code.size();
						argument = 0;
						code.emplace_back(argument, Arg::t_function);
					}
					
					int t = code.size();
					code.emplace_back(Arg::t_eval);
					debug(cout << "-- eval code\n");
					s = parseCode(code, s+1, error_log);
					
					if(!s) return 0;

					code[t].i = code.size() - t - 1;
					state = space;
					argument++;
					s++;
					continue;
				} else if(c == TEMPLATE_START) {
					
					if(iscommand) {
						iscommand = false;
						last_func = code.size();
						argument = 0;
						code.emplace_back(argument, Arg::t_function);
					}
					
					int t = code.size();
					code.emplace_back(Arg::t_template);
					debug(cout << "-- deferred code\n");
					s = parseCode(code, s+1, error_log);
					if(!s) return 0;
					code[t].i = code.size() - t - 1;
					state = space;
					if(iscommand)
						iscommand = false;
					argument++;
					s++;
					continue;
				} 
				/*
				else if(c == '{') {
					debug(cout << "-- expression\n");
					// s = parseExpression(code, s+1);
					if(!s) return 0;
					argument++;
				}
				*/
			}
		}
		
		// --------- triggers ----------
		if(!isescaping && 
			(c == '=' || 
			c == ' ' || 
			c == '\0' || 
			c == EVAL_END || 
			c == TEMPLATE_END || 
			c == FUNCTION_DELIMITER || 
			c == STRING_OPERATOR || 
			c == '\n')) {
				
			if(c=='\n') isnewline = true;
			else isnewline = false;
			
			bool isstring = false;
			if(c == STRING_OPERATOR) {
				if(state == string) {
					state = ident;
					isstring = true;
				} else if(state == space) {
					state = string;
					start = s+1;
				}
			}
			
			if(state == ident) {
				std::string ident(start, (int)(s-start));
				debug(cout << "-- ident: " << ident << endl;);
				Arg a;
				argument++;
				if(isvariable) {
					// its variable
					
					debug(cout << "variable: " << ident << endl);
					
					if(isNumeric(ident)) {
						// check if $1...
						int num_count = 0;
						for(int i=0; i < ident.size(); i++) {
							if(!isdigit(ident[i])) break;
							num_count++;
						}
						if(num_count > 0)
							a = Arg(std::stoi(ident), Arg::t_param);
						if(ident.substr(num_count) == "...") {
							if(num_count == 0)
								a = Arg(-1, Arg::t_param);
							else
								a.i = -a.i;
						}
						// a.dump();
						
					} else {
						// cout << "empty " + ident + "\n";
						if(ident == "") {
							// cout << "empty \n";
							int t=code.size();
							for(; t >= 0 && code[t] != Arg::t_template; t--);
							t++;
							a = Arg(last_func-t, Arg::t_int);
						} else {
							auto it = m_strings.find(ident);
							if(it != m_strings.end()) {
								a = Arg(it->second, Arg::t_variable);
							} else {
								int alloc = alloc_variable(ident);
								a = Arg(alloc, Arg::t_variable);
							}
						}
					}
				} else {
					// not variable
					
					if(!isstring) {
						// try conversions
						// cout << ".......\n";
						if(isNumeric(ident)) {
							int counter = 0;
							for(int i = ident.find('.'); i != std::string::npos; i = ident.find('.',i+1))  counter++;
							if(counter == 1) {
								if(ident.back() == 'f') {
									a = Arg(std::stof(ident), Arg::t_float);
								} else {
									a = Arg(std::stod(ident), Arg::t_double);
								}
							} else if(counter == 0) {
								a = Arg(std::stoi(ident), Arg::t_int);
							} else
								a = Arg(ident, Arg::t_string);
						} else {
							a = Arg(ident, Arg::t_string);
						}
					} else {
						a = Arg(ident, Arg::t_string);
					}
						
					debug(cout << "ident: " << ident << endl);
				}
				
				if(iscommand) {
					// its first argument of eval (command)
					
					iscommand = false;
					// compiletime: (t_function, n_params), (t_int, func index)
					// runtime: (t_function, n_params), (t_string, func_name)
					
					last_func = code.size();
					argument = 0;
					if(ident == GET_VARIABLE_FUNCTION) {
						code.emplace_back(0, Arg::t_get);
					} else if(ident == SET_VARIABLE_FUNCTION) { // known to have 2 params
						code.emplace_back(0, Arg::t_set);
					} else if(ident == IF_FUNCTION) {
						code.emplace_back(0, Arg::t_if);
					} else if(ident == GOTO_FUNCTION) {
						code.emplace_back(0, Arg::t_goto);
					} else {
						argument = 1;
						code.emplace_back(argument, Arg::t_function);
						
						auto it = m_strings.find(ident);
						if(a.type != Arg::t_variable && it != m_strings.end()) {
							code.emplace_back(it->second, Arg::t_int);
						} else {
							if(a.type == Arg::t_int) {
								error_log = "cannot call integer function";
								return 0;
							}
							if(a.type == Arg::t_variable) {
								code[last_func].type = Arg::t_get;
								a.type = Arg::t_int;
							}
							code.push_back(a);
						}
					}
				} else {
					/*
					if(argument == 1) {
						// try to resolve get/set
						if(code[last_func].type == Arg::t_get) {
							
						}
					} else if(argument == 2) {
						
						if(code[last_func].type == Arg::t_set) {
							
						}
					}
					*/
					
					code.push_back(a);
				}
				
				

				state = space;
				isvariable = false;
			}
			
			// handle variable = operator
			#ifdef MUST_BE_VARIABLE_FOR_EQUAL_SET
			if(variable_function) {
			#endif
				// if(c == '=' || (last_func+2 < code.size()) ) {
				if(c == '=' && argument == 1) {
					// code.back(). dump();
					// cout << "------ VARIABLE SET ---- " << endl;
					// if((c == '=' || (last_func+2 < code.size() && code[last_func+2].type == Arg::t_string && code[last_func+2].s == "=")) && code[last_func].type == Arg::t_function) {
					if(code[last_func].type == Arg::t_function || code[last_func].type == Arg::t_get) {
						variable_function = false;
						// if(c != '=') {
							// code.erase(code.end()-1);
							// argument--;
						// }
						// code.back().dump();
						// cout << endl;
						code[last_func].type = Arg::t_set;
						
						state = space;
						isvariable = false;
						
						s++;
						continue;
					}
				}
			#ifdef MUST_BE_VARIABLE_FOR_EQUAL_SET
			}
			#endif
			
			// handle triggers ; and \n
			if((c == FUNCTION_DELIMITER || c == '\n') && state != string) {
				UPDATE_ARGS
				iscommand = true;
				debug(cout << "becomes command\n";)
				debug(cout << endl;)
				state = space;
			}
			
		}
		
		if(isescaping) {
			isescaping = false;
		} else {
			// handle closing triggers
			if(state != string && (c == EVAL_END || c == TEMPLATE_END)) {
				UPDATE_ARGS
				debug(cout << "-- end\n");
				return s;
			}
		}
		
		if(!*s) {
			UPDATE_ARGS
			return s;
		}
		s++;
	}
}

// ---------------------------------------------------------------------
// ----------------------[ VARIABLES AND ALLOCATIONS ]------------------
// ---------------------------------------------------------------------

// executes everything from file
void Command::loadFromFile(std::string filename) {
	std::ifstream file(filename);
	if(!file.good()) return;
	file.seekg (0, file.end);
    int length = file.tellg();
    file.seekg (0, file.beg);

	if(length == 0) return;
	
    char * buffer = new char[length+1];
    file.read(buffer,length);
    std::string commands(buffer,length);
    execute(commands);
    delete[] buffer;
    file.close();
}

void Command::saveVariablesToFile(std::string filename, bool overwrite) {
	
	std::ofstream file(filename, overwrite ? (std::ofstream::out | std::ofstream::trunc) : std::ofstream::app);
    for(auto& variable : m_strings) {
		auto it = m_variables.find(variable.second);
		if(it == m_variables.end()) continue;
		Arg v = m_variables[variable.second];
		std::string var = std::string(SET_VARIABLE_FUNCTION " ") + variable.first + " ";
		switch(v.type) {
			case Arg::t_executable: {
				std::string exec_text = get_executable_text(v);

				Executable& e = m_executables[v.i];
				std::stringstream s;
				decompile_code(s, e.code, 0, e.code.size());
				var += TEMPLATE_START + s.str() + " " + TEMPLATE_END;
				break;
			}
			case Arg::t_int:
				var += std::to_string(v.i);
				break;
			case Arg::t_string:
				var += STRING_OPERATOR + v.s + STRING_OPERATOR;
				break;
			case Arg::t_float:
				var += std::to_string(v.f);
				var += "f";
				break;
			case Arg::t_double:
				var += std::to_string(v.d);
				break;
		}
		file.write(var.c_str(), var.size());
		file.put('\n');
	}
    
	file.close();
}

int Command::alloc_string(const std::string& s) {
	auto it = m_strings.find(s);
	if(it != m_strings.end()) {
		return it->second;
	} else {
		int idx = m_strings.size();
		m_strings[s] = idx;
		m_strings_reverse[idx] = s;
		return idx;
	}
}

int Command::alloc_variable(const std::string& s) {
	auto it = m_strings.find(s);
	if(it != m_strings.end()) {
		return it->second;
	} else {
		int i = alloc_string(s);
		m_variables[i] = Arg();
		add_to_tree(m_root_variables, s);
		return i;
	}
}

std::string Command::get_string(std::string variable) {
	Arg a = get(variable);
	if(a.type == Arg::t_string) {
		return a.s;
	} else {
		return "";
	}
}

Arg Command::get(std::string variable) {
	auto it = m_strings.find(variable);
	if(it != m_strings.end()) {
		return m_variables[it->second];
	}
	Arg a;
	a.type = Arg::t_void;
	return a;
}

void Command::set(std::string variable, const Arg& value) {
	auto it = m_strings.find(variable);
	if(it != m_strings.end()) {
		m_variables[it->second] = value;
	} else {
		m_variables[alloc_variable(variable)] = value;
	}
}

Arg Command::compile(const std::string& command) {
	const char* s = command.c_str();
	std::vector<Arg> code;
	std::string error_log;
	const char* p = parseCode(code, s, error_log);
	Arg a;
	if(!p) { 
		cout << "code parsing failed " << endl;
		cout <<  error_log << endl;
		a = Arg(error_log, Arg::t_string);
		return a;
	}
	debug(cout << "compiled: " << command << endl); 
	debug2(printCompiledCode(code));
	
	Executable exec;
	exec.code = code;
	int idx = m_executables.size();
	exec.id = idx;
	m_executables[idx] = exec;
	a.type = Arg::t_executable;
	a.i = m_executables.size()-1;
	return a;
}

Arg& Command::get_variable(Executable& e, int index) {
	auto it = e.vars.find(index);
	if(it != e.vars.end()) {
		return it->second;
	} else {
		return m_variables[index];
	}
}

// -------------------------------------------------------------------------------------
// ------------------------------------[ EXECUTION CODE ]-------------------------------
// -------------------------------------------------------------------------------------

Arg Command::execute(const Arg& a, const std::vector<Arg>& params, bool global_context) {
	if(a.type != Arg::t_executable) {
		return Arg(Arg::t_void);
	} else {
		int i = 0;
		if(a.i < m_executables.size()) {
			Executable& e = m_executables[a.i];
			return run_executable(e, params, i, e.code.size(), global_context);
		} else {
			cout << "executable not valid: " << a.i << endl;
			return Arg();
		}
	}
}

Arg Command::execute(const std::string& command) {
	Arg a = compile(command);
	std::vector<Arg> args;
	return execute(a,args,true);
}

Arg Command::process_arg(Executable& e, const std::vector<Arg>& params, int& ofs, bool& global_context) {
	int i = ofs;
	const Arg& a = e.code[i];
	switch(a.type) {
		case Arg::t_variable: {
			if(global_context) 
				return m_variables[a.i];
			auto it = e.vars.find(a.i);
			if(it != e.vars.end()) {					
				return it->second;
			} else {
				return m_variables[a.i];
			}
			break;
		}
		case Arg::t_current_executable_reference:
			return Arg(e.id, Arg::t_executable);
			break;
		case Arg::t_template: {
			auto start_code = e.code.begin()+i+1;
			Executable e1;
			e1.vars = e.vars;
			e1.code = std::vector<Arg>(start_code, start_code+a.i);
			int idx = m_executables.size();
			e1.id = idx;
			e1.paused = false;
			e1.instruction_pointer = 0;
			m_executables[idx] = e1;
			ofs += a.i;
			
			debug(
				printCompiledCode(e1.code);
				cout << "from: ";
				start_code->dump();
				cout << " to: ";
				(start_code+a.i-1)->dump();
				cout << endl;
			)
			
			return Arg(idx, Arg::t_executable);
			break;
		}
		case Arg::t_string:
			return a.s;
			break;
		case Arg::t_eval:
			ofs += a.i;
			return run_executable(e, params, i+1, a.i);
			break;
		case Arg::t_param: {
			int param = a.i-1;
			if(param >= 0 && param < params.size()) {
				// debug( params[param].dump(); )
				return params[param];
			} else if(param < 0) {
				
				if(param == -1)
					return Arg((int)params.size());

				debug(cout << "cannot access param " << endl;)
				return Arg(Arg::t_void);
			}
			break;
		}
	}
	return a;
}

bool isfunction(Arg& a) {
	switch(a.type) {
		case Arg::t_function:
		case Arg::t_set:
		case Arg::t_get:
		case Arg::t_if:
		case Arg::t_goto:
			return true;
		default:
			return false;
	}
}

Arg Command::run_executable(Executable& e, const std::vector<Arg>& params, int ofs, int length, bool global_context) {
	int to_ofs = ofs + length;
	
	debug(cout << "starting execution " << e.code.size() << " : " << ofs << " -> " << to_ofs << "\n");
	
	#define SKIP_EXECUTABLE									\
		if(e.code[i].type == Arg::t_eval) {                 \
			i+=e.code[i].i;                                 \
		} else if(e.code[i].type == Arg::t_executable) {    \
			i++;                                            \
		}                                              
	
	int len = 999;
	int c = 0;
	std::vector<Arg> tmp;
	Arg func;
	Arg ret;
go_back:
	for(int i=ofs; i < to_ofs; i++) {
		Arg& a = e.code[i];
		debug(cout << "executing: ");
		debug(a.dump());
		debug(cout << endl);
		
		switch(a.type) {
			// ----------------------- FUNCTION -----------------
			case Arg::t_function: {
				// need to resolve len params
				len = a.i-1;
				
				debug(cout << "executing function " << len << "\n");
				tmp.clear();
				i++;
				Arg& b = e.code[i];
				switch(b.type) {
					case Arg::t_eval:
						i++;
						// func = run_executable(e, params, i, b.i);
						return run_executable(e, params, i, b.i);
						break;
					case Arg::t_string: {
						
						debug(cout << "resolving: " << b.s << endl);
						
						auto it = m_strings.find(b.s);
						if(it != m_strings.end()) {
							func.type = Arg::t_function;
							func.i = it->second;
							debug(cout << "resolved into: " << func.i << endl);
						} else {
							
							debug(cout << "unresolved function: " << b.s << endl);
							
							auto it = m_strings.find(b.s);
							if(it != m_strings.end()) {
								
								debug(cout << "getting variable: " << it->second << endl;)
								
								Arg& v = get_variable(e, it->second);
								if(v.type == Arg::t_string) {
									
									auto it = m_strings.find(b.s);
									if(it != m_strings.end()) {
										func.type = Arg::t_function;
										func.i = it->second;
										
										debug(cout << "resolved into: " << func.i << endl);
										
									} else {
										
										debug(cout << "UNRESOLVED " << b.s << endl;)
										
										return Arg();
									}
									
								} else if(v.type == Arg::t_executable) {
									
									debug(cout << "found executable\n";)
									
									func = v;
								}
							} else {
								debug(cout << "UNRESOLVED " << b.s << endl;)
										
								return Arg();
							}
							
						}
						break;
					}
					case Arg::t_int:
						/*
						cout << "CODE: " << endl;
						printCompiledCode(e.code);
						cout << "PARAMS: " << endl;
						printCompiledCode(params);
						b.dump();
						cout << endl;
						*/
						func = b;
						func.type = Arg::t_function;
						break;
					case Arg::t_executable:
						func = b;
						break;
					case Arg::t_template:
						func = process_arg(e, params, i, global_context);
						break;
				}
				tmp.reserve(len);
				c = 0;
				break;
			}
			// ---------------------- SPECIAL FUNCTIONS --------------------
			/*
			case Arg::t_loop:
				if(ofs == 0) {
					goto go_back;
				} else {
					return a;
				}
				continue;
				break;
			*/
			case Arg::t_if:
				if(a.i >= 2 && a.i <= 3) {
					i++;
					Arg condition = process_arg(e,params,i,global_context);
					// cout << "condition: ";
					// condition.dump();
					// cout << endl;
					
					if(condition.type == Arg::t_int) {
						i++;
						if(condition.i > 0) {
							if(e.code[i].type == Arg::t_eval || e.code[i].type == Arg::t_executable) {
								ret = process_arg(e, params, i, global_context);
							}
							if(a.i == 3) {
								i++;
								SKIP_EXECUTABLE // skip false exec
							}
						} else if(a.i == 3) {
							SKIP_EXECUTABLE // skip true exec
							i++;
							if(e.code[i].type == Arg::t_eval || e.code[i].type == Arg::t_executable) {
								ret = process_arg(e, params, i, global_context);
							}
						} else {
							SKIP_EXECUTABLE // skip true exec
						}
					} else {
						i+=a.i-1;
					}
				} else {
					// runtime error
				}
				break;
			// -------------------------- SET -------------------------------
			case Arg::t_set:
				debug(cout << "executing set\n");
				if(a.i == 2) {
					i++;
					Arg s = process_arg(e,params,i,global_context);//e.code[i+1];
					i++;
					Arg v = process_arg(e,params,i,global_context);
					ret = v;
					if(s.type == Arg::t_string) {
						s.type = Arg::t_int;
						auto it = m_strings.find(s.s);
						if(it != m_strings.end()) {
							debug(cout << "resolving variable: " << s.s << " => " << it->second << endl);
							s.i = it->second;
						} else {
							debug(cout << "unresolved variable: " << s.s << endl);
							// allocate new variable
							s.i = alloc_variable(s.s);
						}
					}
					
					if(s.type == Arg::t_int) {
						if(global_context) {
							auto it = m_variables.find(s.i);
							if(it != m_variables.end()) {
								it->second = v;
							}
						} else {
							auto it = e.vars.find(s.i);
							if(it != e.vars.end()) {
								debug(cout << "setting local variable\n");
								it->second = v;
							} else {
								auto it = m_variables.find(s.i);
								if(it != m_variables.end()) {
									debug(cout << "setting global variable " << s.i << " " << endl);
									// debug(v.dump(););
									// debug(cout << endl;)
									it->second = v;
								} else {
									add_to_tree(m_root_variables, m_strings_reverse[s.i]);
									m_variables[s.i] = v;
								}
							}
						}
						
					}
				}
				break;
			// --------------------------- GET -------------------------
			case Arg::t_get:
				debug(cout << "executing get\n");
				if(a.i == 1) {
					Arg s = e.code[i+1];
					if(s.type == Arg::t_string) {
						s.type = Arg::t_int;
						auto it = m_strings.find(s.s);
						if(it != m_strings.end()) {
							s.i = it->second;
						}
					}
					if(s.type == Arg::t_int) {
						// search local first then global variables
						if(global_context) {
							auto v = m_variables.find(s.i);
							if(v != m_variables.end()) {
								return v->second;
							}
						} else {
							auto it = e.vars.find(s.i);
							if(it != e.vars.end()) {
								return it->second;
							} else {
								auto v = m_variables.find(s.i);
								if(v != m_variables.end()) {
									return v->second;
								}
							}
						}
					}
					i++;
				}
				break;
			case Arg::t_goto:
				i++;
				ret=process_arg(e, params, i, global_context);
				for(i=ret.i; i < e.code.size() && !isfunction(e.code[i]); i++);
				i--;
				cout << "continuing at: " << i << endl;
				continue;
				break;
			// ------------------ RESOLVING ARGUMENTS ---------------------
			case Arg::t_current_executable_reference:
				return Arg(e.id, Arg::t_executable);
			break;
			case Arg::t_param: {
				if(a.i < 0) {
					int arg_start = -(a.i+1);
					if(arg_start < params.size()) {
						// tmp.resize(tmp.size()+params.size()-arg_start);
						for(int j=arg_start; j < params.size(); j++) {
							tmp.push_back(params[j]);
							c++;
						}
					}
				} else {
					tmp.push_back( process_arg(e, params, i, global_context) );
					c++;
				}
				break;
			}
			case Arg::t_template:
			case Arg::t_variable:
			case Arg::t_eval:
				tmp.push_back( process_arg(e, params, i, global_context) );
				c++;
				break;
			
			// ------------------ ADDING ARGUMENTS ------------------------
			default:
				debug(cout << "ident\n";)
				
				tmp.push_back(a);
				c++;
					
				debug(a.dump());
				debug(cout << endl);
				break;
		}
		
		if(c >= len) {
			if(func.type == Arg::t_function) {
				// execute command
				auto it = m_commands.find(func.i);
				
				if(it != m_commands.end()) {
					debug(cout << "calling function: " << m_strings_reverse[func.i] << endl);
					try {
						ret = it->second(tmp);
						debug(printCompiledCode(tmp);)
					} catch(...) {}
				} else {
					auto it = m_variables.find(func.i);
					if(it != m_variables.end()) {
						// Arg execute(const Arg& a, const std::vector<Arg>& args, bool global_context);
						return execute(it->second, tmp, false);
					} else {
						debug(cout << "cannot find command: " << func.i << endl);
						return Arg();
					}
				}
			} else if(func.type == Arg::t_executable) {
				auto it = m_executables.find(func.i);
				if(it != m_executables.end()) {
					run_executable(it->second, tmp, 0, it->second.code.size());
				}
			} else {
				debug(
				cout << "returning ";
				ret.dump();
				cout << endl;
				)
				ret = func;
			}
			len = 999;
			c = 0;
		}
		
	}
	
	if(ofs == 0 && ret.type == Arg::t_loop) {
		goto go_back;
	}
	
	debug(cout << "ending func (result: ";
	ret.dump();cout << "\n");
	return ret;
}

