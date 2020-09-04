/*
	Simple Key binder for SDL2 which executes command bound to key
	
	=============
	COMMAND(void, say, [](std::string msg) {
		std::cout << msg << "\n";
	});
	
	Bind bind;
	bind.SetKey("F1", "say \"hey, im spamming this by F1\"");
	=============
*/
#ifndef BIND_HPP
#define BIND_HPP

#include "Commands.hpp"
#include <unordered_map>
#include <SDL2/SDL.h>

class Bind {
	private:
		std::unordered_map<int, std::pair<Commands::Arg,Commands::Arg>> m_key_executable;
	public:
		Bind();
		~Bind();
		void SaveKeys(std::string filename);
		
		bool GetKeyState(std::string key);
		bool GetKeyState(int key);
		
		bool SetKey(std::string key, Commands::Arg command);
		// bool SetKey(std::string key, int value);
		
		void UnsetKey(std::string key);
		bool OnEvent(SDL_Event& e);
};


#endif
