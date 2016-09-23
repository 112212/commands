#ifndef COMMANDS_HPP
#define COMMANDS_HPP



#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <exception>
#include <functional>
#include <tuple>
#include <set>
#include <type_traits>

// ----- function type information
template< typename t, std::size_t n, typename = void >
struct function_type_information;

template< typename r, typename ... a, std::size_t n >
struct function_type_information< r (*)( a ... ), n > {
	// using type = typename std::tuple_element< n, std::tuple< a ... > >::type; 
	using tuple = std::tuple<a...>;
	using result = r;
};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_type_information< r (c::*)( a ... ), n >
	: function_type_information< r (*)( a ... ), n > {};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_type_information< r (c::*)( a ... ) const, n >
	: function_type_information< r (*)( a ... ), n > {};

template< typename ftor, std::size_t n >
struct function_type_information< ftor, n,
	typename std::conditional< false, decltype( & ftor::operator () ), void >::type >
	: function_type_information< decltype( & ftor::operator () ), n > {};
// --------------------------------------

namespace Commands {

class CommandException {
	private:
		std::string reason;
	public:
	CommandException(std::string s) {
		reason = s;
	}
	std::string& what() { return reason; }
};

struct Arg {
	enum type_enum { t_void, t_int, t_float, t_double, t_string, t_string_ref, // basic types
		   t_end, t_eval, t_template, t_executable, t_variable, 
		   t_function, t_get, t_set, t_if, t_param, t_loop  }
		type;
	
	union {
		int i;
		float f;
		double d;
		std::string s;
	};
	Arg(type_enum type = t_void)  { this->type = type; }
	Arg(int i, type_enum type = t_int) {
		this->type = type;
		this->i = i;
	}
	Arg(float f, type_enum type = t_float) {
		this->type = type;
		this->f = f;
	}
	Arg(double d, type_enum type = t_double) {
		this->type = type;
		this->d = d;
	}
	Arg(const std::string& s, type_enum type = t_string) : s() {
		this->type = type;
		this->s = s;
	}
	Arg& operator=(const Arg& a) {
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
	Arg(const Arg& a) : s() {
		*this = a;
	}

	void dump();
	
	operator int() {
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
	operator std::string() {
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
	~Arg(){}
};

		
// --------- function call: Tuple as arguments
namespace detail
{
    template <typename ret, typename F, typename Tuple, bool Done, int Total, int... N>
    struct call_impl {
        static ret call(F& f, Tuple && t) {
			return call_impl<ret, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    
    template <typename ret, typename F, typename Tuple, int Total, int... N>
    struct call_impl<ret, F, Tuple, true, Total, N...> {
        static ret call(F& f, Tuple && t) {
			return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };
    
}

// user invokes this
template <typename ret, typename F, typename Tuple>
ret call(F& f, Tuple && t) {
    using ttype = typename std::decay<Tuple>::type;
	return detail::call_impl<ret, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}
// -----------------------


struct node;

class Command {
	private:
		
		using adapter_function_type = std::function<Arg(std::vector<Arg>& args)>;
		
		std::unordered_map<int, std::string> m_command_signatures;
		std::unordered_map<int, adapter_function_type> m_commands;
		
		std::unordered_map<std::string, int> m_strings;
		std::unordered_map<int, std::string> m_strings_reverse;
		
		std::unordered_map<int, Arg> m_variables;
		
		struct Executable {
			std::unordered_map<int, Arg> vars;
			std::vector<Arg> code;
		};
		
		std::unordered_map<int, Executable> m_executables;
		
		// -------------------------------------------------------------------------
		// ---------------------- adapter_function_generator -----------------------
		// -------------------------------------------------------------------------
		
		template<bool isvoid, bool done, int n, int N>
		struct adapter_function_generator;
		
		// done = false
		template<bool isvoid, bool done, int n, int N>
		struct adapter_function_generator {
		
			static void handle_element(int& i, std::vector<Arg>& args) {
				if(args[n].type == Arg::t_int) {
					i = args[n].i;
				} else if(args[n].type == Arg::t_string) {
					i = std::stoi(args[n].s);
				}
				else
					throw CommandException("cannot convert to integer");
			}
			static void handle_element(float& f, std::vector<Arg>& args) {
				if(args[n].type == Arg::t_float) {
					f = args[n].f;
				} else if(args[n].type == Arg::t_string) {
					f = std::stof(args[n].s);
				}
			}
			static void handle_element(double& d, std::vector<Arg>& args) {
				d = args[n].d;
			}
			static void handle_element(Arg& a, std::vector<Arg>& args) {
				a = args[n];
			}
			static void handle_element(std::string& s, std::vector<Arg>& args) {
				s = (std::string)args[n];
				if(n+1 == N && args.size() != N) {
					for(int i=n+1; i < args.size(); i++) {
						Arg& a = args[i];
						if(a.type == Arg::t_executable || a.type == Arg::t_void) break;
						s += " " + (std::string)a;
					}
				}
			}
			static void handle_element(std::vector<Arg>& e, std::vector<Arg>& args) {
				e.resize(args.size()-n);
				for(int i=n; i < args.size(); i++) {
					e[i-n] = args[i];
				}
			}

			template<typename F, typename Tuple>
			static Arg adapter_function(F func, std::vector<Arg>& args, Tuple &tuple) {
				using type = typename std::tuple_element<n,Tuple>::type;
				auto& elem = std::get<n>(tuple);
				handle_element(elem, args);
				
				return adapter_function_generator<isvoid, n+1==N, n+1, N>::adapter_function(func, args, tuple);
			}
		};
		
		static void handle_result(Arg& arg, int r) {
			arg.type = Arg::t_int;
			arg.i = r;
		}
		static void handle_result(Arg& arg, float r) {
			arg.type = Arg::t_float;
			arg.f = r;
		}
		static void handle_result(Arg& arg, double r) {
			arg.type = Arg::t_double;
			arg.d = r;
		}
		static void handle_result(Arg& arg, const Arg& a) {
			arg = a;
		}
			
		// done
		template<int n, int N>
		struct adapter_function_generator<false, true, n, N> {
			template<typename F, typename Tuple>
			static Arg adapter_function(F func, std::vector<Arg>& args, Tuple &tuple) {
				using result = typename function_type_information<F,0>::result;
				Arg a;
				handle_result(a, call<result>(func, tuple));
				return a;
			}
			
		};

		template<int n, int N>
		struct adapter_function_generator<true, true, n, N> {
			
			template<typename F, typename Tuple>
			static Arg adapter_function(F func, std::vector<Arg>& args, Tuple &tuple) {
				call<void>(func, tuple);
				return Arg();
			}
			
		};

		// --- signature
		template<bool done, int n, typename Tuple>
		struct signature {
			inline static void get_signature(std::string& s) {
				using type = typename std::tuple_element<n,Tuple>::type;
				if(std::is_same<type, std::string>::value) {
					s += 's'; 
				} else if(std::is_same<type, Arg>::value) {
					s += 'a';
				} else if(std::is_same<type, std::vector<Arg>>::value) {
					s += 'v';
				} else {
					s += typeid(type).name();
				}
				signature<n+1 == std::tuple_size<Tuple>::value, n+1, Tuple>::get_signature(s);
			}
		};
		template<int n, typename Tuple>
		struct signature<true, n, Tuple> {
			inline static void get_signature(std::string& s) {}
		};
		
		// -------------------------------------------------
		// ------------------- adapter ---------------------
		// -------------------------------------------------
		template<bool done, int size, typename Tuple> struct adapter;
		
		template<bool done, int N, typename Tuple>
		struct adapter {
			template<typename F>
			static adapter_function_type make_adapter(F& func) {
				return [=](std::vector<Arg>& vec) -> Arg {
					Tuple tuple;
					using result = typename function_type_information<F,0>::result;
					if(!std::is_same< typename std::tuple_element< N - 1, Tuple >::type, std::vector<Arg> >::value && vec.size() < N) return Arg();
					return adapter_function_generator<std::is_same<result,void>::value, N==0, 0, N>::adapter_function(func, vec, tuple);
				};
			}
		};
		template<int N, typename Tuple>
		struct adapter<true,N,Tuple> {
			template<typename F>
			static adapter_function_type make_adapter(F& func) {
				return [=](std::vector<Arg>& vec) -> Arg {
					Tuple tuple;
					using result = typename function_type_information<F,0>::result;
					return adapter_function_generator<std::is_same<result,void>::value, N==0, 0, N>::adapter_function(func, vec, tuple);
				};
			}
		};
		
		// ---------------------------------------------------------------
		
		const char* parseCode(std::vector<Arg>& code, const char* s, std::string& error_log);
		// const char* parseExpression(std::vector<Arg>& code, const char* s);
		Arg parseVariable(std::vector<Arg>& code, const char*& b);
		
		Arg process_arg(Executable& e, const std::vector<Arg>& params, int &ofs, bool& immediate);
		Arg run_executable(Executable& e, const std::vector<Arg>& params, int ofs, int len, bool immediate = false);
		int alloc_string(const std::string& s);
		int alloc_variable(const std::string& s);
		void add_to_tree(node* root, std::string s);
		node* sweep_node(const std::string& cmd, int cursor);
		void fill(node* n, std::vector<std::string>& str, std::string s, int limit);
		Arg& get_variable(Executable& e, int index);
		void printCompiledCode(std::vector<Arg>& c);
		
		node* m_root_functions;
		node* m_root_variables;
		static Command* singleton;
		
	public:
		Command();
		
		template<typename F>
		bool add_command(const std::string& name, F func) {
			using Tuple = typename function_type_information<F,0>::tuple;
			std::string s;
			signature<std::tuple_size<Tuple>::value==0, 0, Tuple>::get_signature(s);
			int idx = alloc_string(name);
			m_command_signatures[idx] = s;
			m_commands[idx] = adapter< std::tuple_size<Tuple>::value == 0, std::tuple_size<Tuple>::value, Tuple >::make_adapter(func);
			add_to_tree(m_root_functions, name);
		}
		void saveVariablesToFile(std::string filename, bool overwrite = false);
		void loadFromFile(std::string filename);
		const std::string help(const std::string& command);
		
		
		Arg get(std::string variable);
		std::string get_string(std::string variable);
		void set(std::string variable, const Arg& value);
		
		Arg compile(const std::string& command);
		Arg execute(const std::string& command);
		Arg execute(const Arg& a, const std::vector<Arg>& args, bool global_context);
		std::vector<std::string> search(const std::string& cmd, int cursor, int limit = 10);
		std::string complete(const std::string& half_command, int cursor);
		
		// decompile
		std::string get_executable_text(const Arg& arg);
		void decompile_code(std::stringstream& s, std::vector<Arg>& e, int ofs, int len);
		
		
		
		// ----------------------------------------------
		// singleton functions
		static Command& GetSingleton() { return *singleton; }
		static void SetSingleton(Command* r) { singleton = r; }
		
		//
		static void SaveVarariablesToFile(std::string filename, bool overwrite = false) {
			singleton->saveVariablesToFile(filename, overwrite);
		}
		static void LoadFromFile(std::string filename) {
			singleton->loadFromFile(filename);
		}
		//
		
		// --- getting/setting variables
		static Arg Get(const std::string& variable) {
			return singleton->get(variable);
		}
		static std::string GetString(const std::string& variable) {
			return singleton->get_string(variable);
		}
		
		template<typename T>
		static void Set(const std::string& variable, T value) {
			Arg v = Arg(value);
			singleton->set(variable, v);
		}
		//
		
		
		template<typename F>
		static bool AddCommand(const std::string& name, F func) {
			singleton->add_command(name, func);
		}
		
		
		static Arg Execute(const std::string& command) {
			return singleton->execute(command);
		}
		static Arg Execute(const Arg& code, const std::vector<Arg>& args, bool global_context = false) {
			return singleton->execute(code, args, global_context);
		}
		static Arg Compile(const std::string& command) {
			return singleton->compile(command);
		}
		
		
		static std::vector<std::string> Search(std::string command, int cursor, int limit = 10) {
			return singleton->search(command, cursor, limit);
		}
		static const std::string Help(const std::string& command) {
			return singleton->help(command);
		}
		static std::string Complete(const std::string& half_command, int cursor) {
			return singleton->complete(half_command, cursor);
		}
		// -----------------------------------------------
};
} // -- end of namespace
using Commands::Command;

#define COMMAND(ret, name, prototype) \
	ret _cmd_##name prototype; \
	bool _cmd_##name##_cmd_ = Command::AddCommand( #name, (_cmd_##name) ); \
	ret _cmd_##name prototype


#endif
