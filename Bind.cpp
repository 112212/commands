#include "Bind.hpp"
#include <SDL2/SDL.h>
#include <fstream>

#define MWHEEL 5000

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
	{"/", SDLK_SLASH},
	{"esc", SDLK_ESCAPE},
	{"enter", SDLK_RETURN},
	{"lshift", SDLK_LSHIFT},
	{"mwheel", MWHEEL},
	{"mouse1", SDL_BUTTON_LEFT},
	{"mouse2", SDL_BUTTON_RIGHT},
	{"mouse3", SDL_BUTTON_MIDDLE},
	{"mouse4", SDL_BUTTON_X1},
	{"mouse5", SDL_BUTTON_X2},
	
};

#include <iostream>
using namespace std;
using Commands::Arg;

Bind::Bind() {
	if(key_to_str.empty()) {
		for(auto& k : str_to_key) {
			key_to_str[k.second] = k.first;
		}
	}
	Command::AddCommand("bind", [&](std::string key, Arg command) {
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
	Command::AddCommand("unbind", [&](std::string key) {
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


bool Bind::SetKey(std::string key, int value) {
	auto it = str_to_key.find(key);
	if(it != str_to_key.end()) {
		m_key_executable[it->second] = Arg(value);
		return true;
	}
	return false;
}


bool Bind::OnEvent(SDL_Event& e) {
	
	switch(e.type) {
		
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				auto it = m_key_executable.find((int)e.key.keysym.sym);
				if(it != m_key_executable.end() && it->second.type == Arg::t_executable) {
					Arg &code = it->second;
					std::vector<Arg> args {(int)(e.type == SDL_KEYDOWN)};
					Command::Execute(code, args, true);
					return true;
				}
				break;
			}
		case SDL_MOUSEWHEEL:
			{
				auto it = m_key_executable.find(MWHEEL);
				if(it != m_key_executable.end() && it->second.type == Arg::t_executable) {
					Arg &code = it->second;
					std::vector<Arg> args { int(e.wheel.y) };
					Command::Execute(code, args, true);
					return true;
				}
				break;
			}
		
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			{
				auto it = m_key_executable.find(e.button.button);
				if(it != m_key_executable.end() && it->second.type == Arg::t_executable) {
					Arg &code = it->second;
					std::vector<Arg> args { (int)(e.type == SDL_MOUSEBUTTONDOWN) };
					Command::Execute(code, args, true);
					return true;
				}
				break;
			}
			
	}
	
	return false;
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