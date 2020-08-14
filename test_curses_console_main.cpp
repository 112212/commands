#include "CursesConsole.hpp"
#include <iostream>
#include <thread>

CursesConsole console;
int main() {
	console.StartCurses();
	
	console.SetInfoString("hey");
	console.SetCommandPrefix(">> ");
	
	while(1) {
		std::string s = console.Input();
		
		if(s == "quit") {
			break;
		} else if(s.substr(0,4) == "info") {
			// setting empty info string will hide its window
			console.SetInfoString( s.substr(5) );
		} else if(s.substr(0,6) == "prefix") {
			console.SetCommandPrefix( s.substr(7) );
		} else {
			std::cout << "you said: " << s << "\n";
		}
	}
	
	console.StopCurses();
}
