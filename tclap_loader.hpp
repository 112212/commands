/*
	When using TCLAP 
	
	static TCLAP::CmdLine cmd("", ' ', "1.0");
	static TCLAP::ValueArg<std::string> server_ip("", "server_ip", "", false, IP_ANY, "server port");
	static TCLAP::ValueArg<std::string> server_port("", "server_port", "", false, std::to_string(SERVER_PORT), "server port");
	
	int main() {
		// this loads params into Commands system, and they can be saved later
		cmd.add( server_ip );
		cmd.add( server_port );
		try {
			cmd.parse( argc, argv );
		} catch (TCLAP::ArgException &e) {
			return -1;
		}
		
		Commands::TCLAP_LoadVariables(cmd);
		
		std::cout << "server_ip is: " << Command::Get("server_ip").to_string() << "\n";
	}
*/
#ifndef TCLAP_LOADER
#define TCLAP_LOADER

#include <tclap/CmdLine.h>
#include "Commands.hpp"
#include <list>

#include <iostream>

namespace Commands {
void TCLAP_LoadVariables(TCLAP::CmdLine& cmd) {
	std::list<TCLAP::Arg*> args = cmd.getArgList();
	for(auto& a : args) {
		if(a->is_string) {
			TCLAP::ValueArg<std::string>* v = (TCLAP::ValueArg<std::string>*)a;
			if(v) {
				std::string name(a->getName());
				std::string value(v->getValue());
				Command::Set(name, value);
			}
		}
	}
}
}

#endif
