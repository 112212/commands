#include "commands.hpp"
#include <cstring>
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>
#include <sstream>
Command* Command::singleton = new Command();
void arg_convert(Arg& a) {
	if(a.type == Arg::t_string) {
		try {
			a.i = std::stoi(a.s);
			a.type = Arg::t_int;
		} catch(...) {
		}
	}
}

Command::Command() : m_root_node(new node()) {
	
}

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
	// cout << "loading from file: " << length << " : " << commands << endl;
    execute(commands);
    delete[] buffer;
    file.close();
}
void Command::saveVariablesToFile(std::string filename) {
	
	std::ofstream file(filename, std::ofstream::app);

	
    for(auto& variable : m_strings) {
		auto it = m_variables.find(variable.second);
		if(it == m_variables.end()) continue;
		Arg v = m_variables[variable.second];
		std::string var = std::string("set ") + variable.first + " ";
		switch(v.type) {
			case Arg::t_executable: {
				std::string exec_text = get_executable_text(v);

				Executable& e = m_executables[v.i];
				std::stringstream s;
				decompile_code(s, e.code, 0, e.code.size());
				var += "[" + s.str() + " ]";
				break;
			}
			case Arg::t_string:
				var += v.s;
				break;
			case Arg::t_char:
				var += v.c;
				break;
			case Arg::t_float:
				var += v.f;
				var += "f";
				break;
			case Arg::t_double:
				var += v.d;
				break;
		}
		file.write(var.c_str(), var.size());
		file.put('\n');
	}
    
	file.close();
}

#define debug(x) 
#define debug2(x) 

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



char const* type_to_name[50] = { "t_void", "t_int", "t_float", "t_double", "t_char", "t_charp", "t_string", "t_string_ref", // basic types
		   "t_end", "t_eval", "t_template", "t_executable", "t_variable", "t_function", "t_get", "t_set", "t_param", "t_loop", "t_if" };
void Arg::dump() {
	if(type >= 19 || type < 0) return;
	cout << "(" << type_to_name[(int)type] << ", ";
	switch(type) {
		case t_float:
			cout << f; break;
		case t_double:
			cout << d; break;
		case t_char:
			cout << c; break;
		case t_string:
			cout << '"' << s << '"'; break;
		default:
			cout << i; break;
	}
	cout << ")";
}


void Command::printCompiledCode(std::vector<Arg>& code) {
	cout << code.size() << " code: ";
	for(auto& a : code) {
		a.dump();
		cout << ", ";
	}
	cout << endl;
}


void Command::decompile_code(std::stringstream& s, std::vector<Arg>& e, int ofs, int len) {
	bool first = true;
	len += ofs;
	debug2(cout << "decompiling" << endl;)
	for(int i=ofs; i < len; i++) {
		Arg& a = e[i];
		switch(a.type) {
			case Arg::t_template:
				debug2(cout << "template: ";
				a.dump();
				cout << endl;)
				
				s << " [";
				i++;
				decompile_code(s, e, i, a.i);
				i+=a.i-1;
				s << " ]";
				break;
			case Arg::t_eval:
				debug2(cout << "evaluating: ";
				a.dump();
				cout << endl;)
				
				s << " (";
				i++;
				decompile_code(s, e, i, a.i);
				i+=a.i-1;
				s << ")";
				break;
			case Arg::t_loop:
				s << "; $";
				break;
			case Arg::t_if:
			case Arg::t_get:
			case Arg::t_set:
			case Arg::t_function: {
				if(!first) {
					s << ";";
				}
				if(a.type == Arg::t_if) {
					s << " if";
				} else if(a.type == Arg::t_get) {
					s << " get";
				} else if(a.type == Arg::t_set) {
					s << " set";
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
				s << " $" << m_strings_reverse[a.i];
				break;
			}
			case Arg::t_int:
				s << " " << a.i;
				break;
			case Arg::t_float:
				s << a.f;
				s << "f ";
				break;
			case Arg::t_char:
				s << " " << a.c;
				break;
			case Arg::t_string:
				s << " " << a.s;
				break;
			case Arg::t_double:
				s << " " << a.d;
				break;
		}
	}
}

static bool isIdent(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

void Command::add_to_tree(std::string s) {
	node* n = m_root_node;
	for(auto& cc : s) {
		int c = tolower(cc);
		if(c < 'a' || c > 'z') return;
		c -= 'a';
		
		if(!n->nodes[c]) {
			n->nodes[c] = new node();
		}
		
		n = n->nodes[c];
	}
	n->cmd = alloc_string(s);
}




std::vector<std::string> Command::search(std::string command) {
	// to implement
}

std::string Command::complete(const std::string& cmd, int cursor) {
	int i;
	for(i=cursor; i > 0; i--) {
		if(cmd[i] == '[' || cmd[i] == '(') break;
	}
	
	if(i < 0) i = 0;
	int start=-1, stop=cmd.size();
	for(; i < cmd.size(); i++) {
		if(isIdent(cmd[i])) {
			start = i;
			break;
		}
	}
	for(; i < cmd.size(); i++) {
		if(!isIdent(cmd[i])) {
			stop = i;
			break;
		}
	}
	if(start == -1 || stop == -1) return "";
	std::string ident = cmd.substr(start, stop-start);
	
	node* n = m_root_node;
	
	for(i=0; i < ident.size(); i++) {
		int c = tolower(ident[i]);
		if(c < 'a' || c > 'z') return "";
		c -= 'a';
		if(n) {
			n = n->nodes[c];
		} else
			return "";
	}
	
	std::string additional = "";
	while(n) {
		if(n->cmd != -1) return additional;
		node* d = nullptr;
		char ch = 'a';
		for(node* c : n->nodes) {
			if(c) {
				if(!d) {
					d = c;
					additional += ch;
				} else {
					return additional;
				}
			}
			ch++;
		}
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
				case 'c': ret += "char "; break;
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


static bool isNumeric(std::string& s) {
	for(int i=0; i < s.size(); i++) {
		if(s[i] < '0' || s[i] > '9') return false;
	}
	return true;
}

const char* Command::parseCode(std::vector<Arg>& code, const char* s, std::string& error_log) {
	bool isescaping = false;
	bool iscommand = true;
	bool isvariable = false;
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
			code[last_func].i = argument; \
		}
		
	const char * start = s;
	Arg a;
	while(s) {
		const char &c = *s;
		
		if(state != string) {
			
			if(isIdent(c) || c == '\\') {
				
				if(state == space) {
					start = s;
					state = ident;
				}
				if(c == '\\') {
					debug(cout << "escaping\n");
					isescaping = true;
					s++;
					start++;
					continue;
				}
				
				
			} else if(!isescaping) {
				 if(c == '$') {
					if(!isIdent(s[1])) {
						if(iscommand) {
							iscommand = false;
							last_func = code.size();
							argument = 0;
							code.emplace_back(Arg::t_loop);
						} else {
							error_log = std::string(s, std::min<int>(std::strlen(s), 10)) + " after $ must go identifier, like: $identifier";
							return 0;
						}
					} else {
						isvariable = true;
					}
				} else if(c == '(') {
					/*
					if(isvariable) {
						code.emplace_back(Arg::t_eval);
					}
					*/
					
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
					/*
					if(isvariable) {
						isvariable = false;
					}
					*/
					code[t].i = code.size() - t - 1;
					state = space;
					argument++;
					s++;
					continue;
				} else if(c == '[') {
					// compile template
					
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
				} else if(c == '{') {
					debug(cout << "-- expression\n");
					s = parseExpression(code, s+1);
					if(!s) return 0;
					argument++;
				}
			}
		}
		if(!isescaping && (c == ' ' || c == '\0' || c == ')' || c == ']' || c == ';' || c == '"' || c == '\n')) {
			
			if(c == '\"') {
				if(state == string) {
					state = ident;
					// argument++;
				} else if(state == space) {
					state = string;
					start = s+1;
				}
			}
				
			if(state == ident) {
				std::string ident(start, (int)(s-start));
				Arg a;
				argument++;
				if(isvariable) {
					debug(cout << "variable: " << ident << endl);
					isvariable = false;
					
					if(isNumeric(ident)) {
						a = Arg(std::stoi(ident), Arg::t_param);
					} else {
					
						auto it = m_strings.find(ident);
						if(it != m_strings.end()) {
							a = Arg(it->second, Arg::t_variable);
						} else {
							int alloc = alloc_variable(ident);
							m_variables[alloc] = Arg();
							a = Arg(alloc, Arg::t_variable);
						}
					}
				} else {
					if(isNumeric(ident)) {
						a = Arg(std::stoi(ident));
					} else
						a = Arg(ident, Arg::t_string);
					debug(cout << "ident: " << ident << endl);
				}
				
				if(iscommand) {
					iscommand = false;
					// compiletime: (t_function, n_params), (t_int, func index)
					// runtime: (t_function, n_params), (t_string, func_name)
					
					last_func = code.size();
					if(ident == "get") {
						argument = 0;
						code.emplace_back(0, Arg::t_get);
					} else if(ident == "set") { // known to have 2 params
						argument = 0;
						code.emplace_back(0, Arg::t_set);
					} else if(ident == "if") {
						argument = 0;
						code.emplace_back(0, Arg::t_if);
					} else {
						argument = 1;
						code.emplace_back(argument, Arg::t_function);
						auto it = m_strings.find(ident);
						if(it != m_strings.end()) {
							code.emplace_back(it->second, Arg::t_int);
						} else
							code.push_back(a);
					}
				} else {
					if(argument == 1) {
						// try to resolve get/set
						if(code[last_func].type == Arg::t_get) {
							
						} else if(code[last_func].type == Arg::t_set) {
							
						}
					}
					code.push_back(a);
				}
				
				
				state = space;
				
			}
			
			
			if((c == ';' || c == '\n') && state != string) {
				UPDATE_ARGS
				iscommand = true;
				debug(cout << "becomes command\n";)
				cout << endl;
				state = space;
			}
			
		}
		
		if(isescaping) {
			isescaping = false;
		} else {
			if(state != string && (c == ')' || c == ']')) {
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

const char* Command::parseExpression(std::vector<Arg>& code, const char* s) {
	// to implement
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
		return i;
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

void Command::set(std::string variable, Arg value) {
	auto it = m_strings.find(variable);
	if(it != m_strings.end()) {
		m_variables[it->second] = value;
	} else {
		m_strings[variable] = m_variables.size();
		m_variables[m_variables.size()] = value;
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
	m_executables[m_executables.size()] = (exec);
	a.type = Arg::t_executable;
	a.i = m_executables.size()-1;
	return a;
}



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
		case Arg::t_template: {
			auto start_code = e.code.begin()+i+1;
			Executable e1;
			e1.vars = e.vars;
			e1.code = std::vector<Arg>(start_code, start_code+a.i);
			int idx = m_executables.size();
			m_executables[idx] = e1;
			ofs += a.i;
			
			debug(
				printCompiledCode(e1.code);
				cout << "from: ";
				start_code->dump();
				cout << " to: ";
				(start_code+a.i)->dump();
				cout << endl;
			)
			
			return Arg(idx, Arg::t_executable);
			break;
		}
		case Arg::t_eval:
			ofs += a.i;
			return run_executable(e, params, i+1, a.i);
			break;
		case Arg::t_param: {
			int param = a.i-1;
			if(param >= 0 && param < params.size()) {
				// debug( params[param].dump(); )
				return params[param];
			} else if(param == -1) {
				return Arg((int)params.size());
			} else {
				debug(cout << "cannot access param " << endl;)
				return Arg(0);
			}
			break;
		}
	}
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

Arg Command::run_executable(Executable& e, const std::vector<Arg>& params, int ofs, int length, bool global_context) {
	int to_ofs = ofs + length;
	
	debug(cout << "starting execution " << e.code.size() << " : " << ofs << " -> " << to_ofs << "\n");
	
	#define SKIP_EXECUTABLE									\
		if(e.code[i].type == Arg::t_eval) {                 \
			i+=e.code[i].i;                                 \
		} else if(e.code[i].type == Arg::t_executable) {    \
			i++;                                            \
		} else {                                            \
			cout <<"FAIL\n";                                \
			exit(0);                                        \
		}                                                   
	
	int len = 999;
	int c = 0;
	std::vector<Arg> tmp;
	Arg func;
	Arg ret;
go_back:
	for(int i=ofs; i < to_ofs; i++) {
		Arg& a = e.code[i];
		debug(cout << "parsing: ");
		debug(a.dump());
		debug(cout << endl);
		
		switch(a.type) {
			// ----------------------- FUNCTION -----------------
			case Arg::t_function: {
				// need to resolve len params
				len = a.i-1;
				
				debug(cout << "executing function " << len << "\n");
				
				i++;
				Arg& b = e.code[i];
				switch(b.type) {
					case Arg::t_eval:
						i++;
						func = run_executable(e, params, i, b.i);
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
				tmp.resize(len);
				c = 0;
				break;
			}
			// ---------------------- SPECIAL FUNCTIONS --------------------
			case Arg::t_loop:
				if(ofs == 0) {
					goto go_back;
				} else {
					return a;
				}
				continue;
				break;
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
			
			// ------------------ RESOLVING ARGUMENTS ---------------------
			case Arg::t_param:
			case Arg::t_template:
			case Arg::t_variable:
			case Arg::t_eval:
				// cout << "here\n";
				if(c < tmp.size())
					tmp[c++] = process_arg(e, params, i, global_context);

				// cout << "here2\n";
				break;
			
			// ------------------ ADDING ARGUMENTS ------------------------
			default:
				debug(cout << "ident\n";)
				if(c < tmp.size())
					tmp[c++] = a;
				debug(a.dump());
				debug(cout << endl);
				break;
		}
		
		if(c >= len) {
			if(func.type == Arg::t_function) {
				// execute command
				auto it = m_commands.find(func.i);
				
				if(it != m_commands.end()) {
					debug(cout << "executing: " << func.i << endl);
					try {
						ret = it->second(tmp);
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
				cout << "returning ";
				ret.dump();
				cout << endl;
				ret = func;
			}
			len = 999;
			c = 0;
		}
		
	}
	
	if(ofs == 0 && ret.type == Arg::t_loop) {
		// cout << "going back\n";
		goto go_back;
	}
	
	debug(cout << "ending func\n");
	return ret;
}

Arg Command::execute(const std::string& command) {
	Arg a = compile(command);
	std::vector<Arg> args;
	return execute(a,args,true);
}
