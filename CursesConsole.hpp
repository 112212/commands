/*
	Useful console for making network server or live interpreter
	
	Features:
		- live log window (std::cout is redirected on cout)
			- scroll log window with: PAGE UP and PAGE DOWN
		- info string that sits on top always
		- command prefix (show current dir for example)
		- command history (ARROW UP and ARROW DOWN scrolls history commands)
		- command completion
		
	// this will display in info string possible commands to be completed, and will also autocomplete
	console.SetCodeCompleteHandler( [](std::string cmd, int cursor) {
		const int max_commands_to_complete = 5;
		std::vector<std::string> vec = Command::Search(cmd, cursor, max_commands_to_complete);
		if(!vec.empty()) {
			std::string info = "";
			for(auto& s : vec) {
				info += s + " | ";
			}
			console.SetInfoString(info);
		}
		return Command::Complete(cmd, cursor);
	});
*/

#ifndef CURSES_CONSOLE_HPP
#define CURSES_CONSOLE_HPP

#include <iostream>
#include <vector>
#include <string>


struct _win_st;
typedef struct _win_st WINDOW;
struct ncurses_stream;

class CursesConsole {
	public:
		CursesConsole();
		void Refresh();
		void Print(std::string str);
		void Print(char ch);
		std::string Input();
		void StartCurses();
		void StopCurses();
		void SetCodeCompleteHandler( std::string (*handler)(std::string cmd, int cursor) );
		void SetAutoCodeComplete(bool value);
		void SetInfoString(std::string str);
		void SetTmpInfoString(std::string str);
		
		void SetRefreshOnInput(bool value);
		void SetCommandPrefix(std::string prefix);
		std::string GetCommand();
		std::string GetInfoString();
		void ClearLog();
	private:
		std::string (*code_complete_handler)(std::string cmd, int cursor);
		int get_log_height();
		int get_info_height();
		friend struct ncurses_stream;
		WINDOW* m_window;
		ncurses_stream* m_stream;
		std::vector<std::string> m_lines;
		
		// command and history
		std::string m_command;
		std::string m_last_command;
		std::string m_command_prefix;
		int m_command_cursor;
		int m_history_counter;
		std::vector<std::string> m_history;
		
		std::string m_info_string;
		std::string m_tmp_info_string;
		
		int m_window_height, m_window_width;
		int m_cursor; // only y dimension
		
		bool active;
		bool dirty;
		bool m_auto_refresh;
		bool m_auto_code_complete;
};

#endif
