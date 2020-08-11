#include <iostream>
using namespace std;
#include "commands.hpp"
using Commands::Arg;
using Commands::Object;
COMMAND(int, inc, (int a, int b)) {
	// cout << "func: " << a << ", " << b << endl;
	return a + b;
}
COMMAND(int, lt, (int a, int b)) {
	// cout << "func: " << a << ", " << b << endl;
	return a < b;
}
COMMAND(int, gt, (int a, int b)) {
	// cout << "func: " << a << ", " << b << endl;
	return a > b;
}

COMMAND(std::string, lol, (std::vector<Arg> args)) {
	if(args.size() < 2) return "fail";
	if(args[0].type != Arg::t_string) return "1st param";
	if(args[1].type != Arg::t_string) return "2nd param";
	// cout << "lol: " << args[0].s << ", " << args[1].s << endl;
	return "hehehe";
}

COMMAND(int, str_func, (std::string a, std::string b)) {
	cout << "str_func: " << a << ", " << b << endl;
	return 0;
}
COMMAND(std::string, hehe, (std::string a)) {
	// cout << "AWESOME hehe: " << a << endl;
	return "Cool Right :D";
}
COMMAND(int, print, (std::vector<Arg> args)) {
	cout << "print: ";
	for(auto& a : args) {
		if(a.type == Arg::t_string)
			cout << a.s;
		else
			cout << a.i;
		cout << " ";
	}
	cout << endl;
	return 0;
}

COMMAND(int, lolzy, (int a)) {
	cout << "l" << a << endl;
	return 5;
}

COMMAND(int, loop, (int times, Arg code)) {
	if(code.type == Arg::t_executable) {
		std::vector<Arg> args;
		for(int i=0; i < times; i++) {
			Command::Execute(code, args);
		}
	}
	return 5;
}



COMMAND(int, variable_args, (std::string a, std::string b, std::vector<Arg> args)) {
	cout << "veriable_args: " << a << ", " << b << ", given additional args: " << args.size() << endl;
	for(auto& arg : args) {
		if(arg.type == Arg::t_string) {
			cout << "arg: " << arg.s << endl;
		}
	}
	return 0;
}

#include <cstdio>
void test() {
	// cout << "loool" << endl;
	printf("looool\n");
}

#include <chrono>

struct my_vec3 {
	float x,y,z;
};
Commands::ObjectInfo obj_vector{.type=typeid(my_vec3)};



void vec_destroy(void* ptr) {
	std::cout << "destroying vector\n";
	delete static_cast<my_vec3*>(ptr);
}


int main() {
	Command::AddCommand("w", [](std::string nick, std::string message) -> int {
		cout << "nick: " << nick << ", message: " << message << endl;
		return 0;
	});
	
	Command::AddCommand("vec3", [](float x, float y, float z) -> Object {
		std::cout << "creating vector: " << x << ","<<y << "," << z << "\n";
		auto *v = new my_vec3;
		v->x = x;
		v->y = y;
		v->z = z;
		Object o(&obj_vector, v);
		std::cout << "on creation: " << &obj_vector << "\n";
		return o;
	});
	
	Command::AddCommand("vec3p", [](Arg o) -> void {
		auto *v = o.to_object<my_vec3>();
		
		// if(o.type == Arg::t_object) {
			// if(o.o.type == &obj_vector) {
				// auto &v = *static_cast<my_vec3*>(o.o.ptr);
		if(v) {
			std::cout << "vec3p : " << v->x << ", " << v->y << ", " << v->z << "\n";
		} else {
			std::cout << "vec3p returned null\n";
		}
	});
	
	cout << "compiling: \n";
	std::vector<Arg> args;
	
	auto tp = std::chrono::high_resolution_clock::now();
	// Arg a = Command::Compile("set myfunc [ hehe $1 ]; myfunc \"FUCK YEA :D\"; set yeah \"very awesome\"; loop 10 [ hehe $yeah ] ; set hehe \"very $cool (hehe haha) ; awesome :D nice\"; lol $hehe 5 6 (hehe troll (get hehe) $hehe 66 hehe (lolzy (hehe lelele)));set hehe 5");
	// Arg a = Command::Compile("set a 5; set lolzyfunc [ print $1 $3 $0; loop 1 [ print lol $a ]; set a (inc $a 1); if (lt $a 500) ($) (print njahahaha; $; print \"DONE :D\") ]; lolzyfunc heheh very nice :D; loop 10 [ hehe $yeah ]; set hehe 5");
	Arg a = Command::Compile("set a 5; set lolzyfunc [ print $1 $3 $0; loop 1 [ print lol $a ]; ]");
	// for(int i=0; i < 11695; i++)
		// test();
		Command::Execute(a,args);
	std::chrono::duration<float, std::ratio<1, 1000000>> duration = (std::chrono::high_resolution_clock::now() - tp);
	cout << "time: " << duration.count()<< endl;
	
	// cout << Command::Help("set") << endl;
	
	cout << "variable: " << Command::Get("hehe") << endl;
	// return 0;
	
	try {
		Command::Execute("vec3p (vec3 1.32 3 5.7)");
	} catch(...) {
		cout << "vec3 exception\n";
	}
	
	try {
		Command::Execute("func hehe haha");
	} catch(...) {
		cout << "func command exception1\n";
	}
	
	try {
		Command::Execute("func 15 haha");
	} catch(...) {
		cout << "func command exception2\n";
	}
	
	try {
		Command::Execute("func 15 65");
	} catch(...) {
		cout << "func command exception3\n";
	}
	
	try {
		Command::Execute("variable_args 15 65");
	} catch(...) {
		cout << "variable_args command exception1\n";
	}
	
	try {
		Command::Execute("variable_args 15 65 dfgfd gff hghf 15 25");
	} catch(...) {
		cout << "variable_args command exception2\n";
	}
	
	Command::Execute("str_func hehe haha");
	
	try {
		Command::Execute("w Nick bla bla bla hehehe, very nice :D");
	} catch(...) {}
	
	return 0;
}
