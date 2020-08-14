#include "CursesConsole.hpp"
#include "Commands.hpp"
#include <iostream>
#include <thread>

using namespace Commands;
using Commands::Arg;

CursesConsole console;

COMMAND(int, print, (std::vector<Arg> args)) {
	std::cout << "print: ";
	for(auto& a : args) {
		std::cout << a.to_string();
		std::cout << " ";
	}
	std::cout << std::endl;
	return 0;
}

COMMAND(void, info, (std::string str)) {
	console.SetInfoString(str);
}

COMMAND(void, clear, ()) {
	console.ClearLog();
}


int main() {
	console.StartCurses();
	
	console.SetCommandPrefix(">> ");
	
	// calls code complete handler after each keystroke (but doesn't actually autocomplete)
	console.SetAutoCodeComplete(true);
	
	// this will display in info string possible commands to be completed, and will also autocomplete
	console.SetCodeCompleteHandler( [](std::string cmd, int cursor) {
		const int max_commands_to_complete = 5;
		std::vector<std::string> vec = Command::Search(cmd, cursor, max_commands_to_complete);
		if(!vec.empty()) {
			std::string info = "";
			for(auto& s : vec) {
				info += s + " | ";
			}
			console.SetTmpInfoString(console.GetInfoString() + "\n------\n" + info);
		}
		return Command::Complete(cmd, cursor);
	});
	
	while(1) {
		console.SetInfoString(
			"enter:"
				"\n\tprint helloworld"
				"\n\tprint 3 (+ 45 2 (- 7 2) 8 (* 2 5)) (/ 10 2.5)"
				"\n\tprint (/f 10.6 2.57)"
				"\n\tclear"
		);
	
		std::string s = console.Input();
		
		try {
			Command::Execute(s);
		} catch(std::exception &e) {
			std::cout << e.what() << "\n";
		}
	}
	
	console.StopCurses();
}
