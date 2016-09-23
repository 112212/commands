#ifndef BIND_HPP
#define BIND_HPP

#include "commands.hpp"
#include <unordered_map>
#include <SDL2/SDL.h>

class Bind {
	private:
		std::unordered_map<int, Commands::Arg> m_key_executable;
		
	public:
		Bind();
		~Bind();
		void SaveKeys(std::string filename);
		bool SetKey(std::string key, std::string command);
		void UnsetKey(std::string key);
		bool OnEvent(SDL_Event& e);
};


#endif
