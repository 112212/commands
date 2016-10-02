#ifndef TCLAP_LOADER
#define TCLAP_LOADER

#include <tclap/CmdLine.h>
#include "commands.hpp"
#include <list>

#include <iostream>

namespace Commands {
void LoadVariables(TCLAP::CmdLine& cmd) {
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
