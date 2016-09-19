#include "bind.hpp"
#include <SDL2/SDL.h>
#include <fstream>

static std::unordered_map<int, std::string> key_to_str;
static std::unordered_map<std::string, int> str_to_key = {
	{"F1", SDLK_F1},
	{"F2", SDLK_F2},
	{"F3", SDLK_F3},
	{"F4", SDLK_F4},
	{"F5", SDLK_F5},
	{"F6", SDLK_F6},
	{"F7", SDLK_F7},
	{"F8", SDLK_F8},
	{"F9", SDLK_F9},
	{"a", SDLK_a},
	{"a", SDLK_a},
	{"b", SDLK_b},
	{"c", SDLK_c},
	{"d", SDLK_d},
	{"e", SDLK_e},
	{"f", SDLK_f},
	{"g", SDLK_g},
	{"h", SDLK_h},
	{"i", SDLK_i},
	{"j", SDLK_j},
	{"k", SDLK_k},
	{"l", SDLK_l},
	{"m", SDLK_m},
	{"n", SDLK_n},
	{"o", SDLK_o},
	{"p", SDLK_p},
	{"q", SDLK_q},
	{"r", SDLK_r},
	{"s", SDLK_s},
	{"t", SDLK_t},
	{"u", SDLK_u},
	{"v", SDLK_v},
	{"w", SDLK_w},
	{"x", SDLK_x},
	{"y", SDLK_y},
	{"z", SDLK_z},
	
};

#include <iostream>
using namespace std;

Bind::Bind() {
	if(key_to_str.empty()) {
		for(auto& k : str_to_key) {
			key_to_str[k.second] = k.first;
		}
	}
	Command::AddCommand("bind", [&](std::string key, Arg command) -> int{
		auto it = str_to_key.find(key);
		if(it != str_to_key.end()) {
			if(command.type == Arg::t_executable) {
				m_key_executable[it->second] = command;
			} else if(command.type == Arg::t_string) {
				Arg code = Command::Compile(command.s);
				m_key_executable[it->second] = code;
			}
		}
	});
	Command::AddCommand("unbind", [&](std::string key) -> int{
		UnsetKey(key);
	});
}
Bind::~Bind() {}
void Bind::UnsetKey(std::string key) {
	auto it = str_to_key.find(key);
	if(it != str_to_key.end()) {
		m_key_executable.erase(it->second);
	}
}
bool Bind::SetKey(std::string key, std::string command) {
	auto it = str_to_key.find(key);
	if(it != str_to_key.end()) {
		Arg code = Command::Compile(command);
		if(code.type == Arg::t_executable) {
			m_key_executable[it->second] = code;
		}
		return true;
	}
	return false;
}


void Bind::OnEvent(SDL_Event& e) {
	
	if(e.type == SDL_KEYDOWN) {
		auto it = m_key_executable.find((int)e.key.keysym.sym);
		if(it != m_key_executable.end()) {
			Arg &code = it->second;
			std::vector<Arg> args;
			Command::Execute(code, args, true); 
		}
	}
}

void Bind::SaveKeys(std::string filename) {
	std::ofstream file(filename);
	for(auto& i : m_key_executable) {
		Arg a = i.second;
		std::string text = std::string("bind ") + key_to_str[i.first] + " [ " + Command::GetSingleton().get_executable_text(a) + " ]";
		file.write(text.c_str(), text.size());
		file.put('\n');
	}
	file.close();
}
