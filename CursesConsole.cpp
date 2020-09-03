#include "CursesConsole.hpp"
#include <iostream>
#include <string>
#include <ncurses.h>

// using namespace curses;

class ncursesbuf: public std::streambuf {
	public:
		void SetConsole(CursesConsole* cons) { this->cons = cons; }
		ncursesbuf() { cons = 0; }
		virtual int overflow(int c) {
			cons->Print((char)c);
			return 1;
		}
	private:
		CursesConsole* cons;
};

class ncurses_stream : public std::ostream {
	public:
		ncursesbuf tbuf;
		std::ostream &src;
		std::streambuf* const old_buf;

		ncurses_stream(CursesConsole* cons, std::ostream &o) : 
			src(o), 
			old_buf(o.rdbuf()),
			std::ostream(&tbuf) 
		{
			tbuf.SetConsole(cons);
			o.rdbuf(rdbuf());
		}

		~ncurses_stream() {
			src.rdbuf(old_buf);
		}
};

void CursesConsole::StartCurses() {
	initscr();
	cbreak();
	int h,w;
	w = COLS;
	h = LINES;
	m_window = newwin(h,w,0,0);
	keypad(m_window, true);
	if(!m_stream)
		m_stream = new ncurses_stream(this, std::cout);
	m_window_width = w;
	m_window_height = h;
	m_lines.emplace_back("");
	m_command_prefix = "";
	m_command = m_command_prefix;
	m_command_cursor = m_command_prefix.size();
	m_auto_refresh = true;
	m_auto_code_complete = false;
}


void CursesConsole::SetAutoCodeComplete(bool value) {
	m_auto_code_complete = value;
}

void CursesConsole::StopCurses() {
	if(m_stream) {
		delete ((ncurses_stream*)m_stream);
		m_stream = 0;
	}
	endwin();
}

void CursesConsole::SetRefreshOnInput(bool value) {
	m_auto_refresh = value;
}

void CursesConsole::SetCommandPrefix(std::string prefix) {
	m_command = prefix + GetCommand();
	m_command_prefix = prefix;
	m_command_cursor = std::min<int>( std::max<int>(m_command_prefix.size(), m_command_cursor), m_command.size() );
	dirty = true;
	if(m_auto_refresh) {
		Refresh();
	}
}

void CursesConsole::ClearLog() {
	m_lines.clear();
	m_cursor = 0;
	dirty = true;
	if(m_auto_refresh) {
		Refresh();
	}
}

void CursesConsole::SetInfoString(std::string str) {
	m_info_string = str;
	dirty = true;
	if(m_auto_refresh) {
		Refresh();
	}
}

void CursesConsole::SetTmpInfoString(std::string str) {
	m_tmp_info_string = str;
	dirty = true;
	if(m_auto_refresh) {
		Refresh();
	}
}

std::string CursesConsole::GetInfoString() {
	return m_info_string;
}

int CursesConsole::get_log_height() {
	std::string info_strings = m_info_string+m_tmp_info_string;
	return m_window_height-1-m_command.size()/m_window_width-1 - get_info_height(&info_strings);
}

int CursesConsole::get_info_height(std::string* info_string) {
	if(!info_string) info_string = &m_info_string;
	if(info_string->empty()) return 0;
	std::string::size_type prev=0;
	int lines = 1;
	for(auto it = info_string->find_first_of('\n'); ; 
		prev=it, it = info_string->find_first_of('\n', it+1)) {
		lines += (std::min(info_string->size(), it) - prev + 1) / m_window_width + 1;
		if(it == std::string::npos) {
			break;
		}
	}
	return lines;
}

void CursesConsole::Refresh() {
	if(dirty && m_window) {
		dirty = false;
		wclear(m_window);
		
		int curs = 0;
		int log_height = get_log_height();
		for(auto it = m_lines.begin()+m_cursor; curs < log_height && it != m_lines.end(); it++) {
			wmove(m_window, curs++, 0);
			wprintw(m_window, it->c_str());
		}
		
		std::string info_strings = m_info_string+m_tmp_info_string;
		int info_height = get_info_height(&info_strings);
		// int tmp_info_height = get_info_height(&m_tmp_info_string);
		
		if(info_height > 0) {
			std::string info_string = !m_tmp_info_string.empty() ? (m_info_string + m_tmp_info_string) : m_info_string;
			mvwhline(m_window, log_height, 0, ACS_HLINE, m_window_width);
			mvwprintw(m_window, log_height+1, 0, info_string.c_str());
			// mvwhline(m_window, log_height+info_height, 0, ACS_HLINE, m_window_width);
		}
		
		mvwhline(m_window, log_height+info_height, 0, ACS_HLINE, m_window_width);
		mvwprintw(m_window, log_height+info_height+1, 0, m_command.c_str());
		wmove(m_window, log_height+info_height+1, m_command_cursor);
		wrefresh(m_window);
	}
}

void CursesConsole::Print(std::string str) {
	for(auto c : str) {
		Print(c);
	}
	dirty = true;
}

void CursesConsole::Print(char ch) {
	if(ch == '\n')
		m_lines.emplace_back("");
	else {
		if(m_lines.back().size() >= m_window_width)
			m_lines.emplace_back("");
		m_lines.back() += ch;
	}
	int log_height = get_log_height();
	if(m_lines.size() > log_height) {
		m_cursor = m_lines.size() - log_height - 1;
	}
	dirty = true;
}

CursesConsole::CursesConsole() {
	dirty = false;
	m_stream = 0;
	m_window = 0;
	m_cursor = 0;
	code_complete_handler = 0;
	m_history_counter = 0;
}

void CursesConsole::SetCodeCompleteHandler( std::string (*handler)(std::string cmd, int cursor) ) {
	code_complete_handler = handler;
}

std::string CursesConsole::GetCommand() {
	return m_command.substr(m_command_prefix.size());
}

std::string CursesConsole::Input() {
	while(1) {
		if(m_auto_refresh) {
			Refresh();
		}
		if(!m_tmp_info_string.empty()) {
			m_tmp_info_string.clear();
		}
		int input = wgetch(m_window);
		switch(input) {
			case KEY_UP:
				if(m_history_counter == 0) 
					break;
				if(m_history_counter == m_history.size()) {
					m_last_command = m_command;
				}
				m_command = m_history[--m_history_counter];
				m_command_cursor = m_command.size();
				break;
			case KEY_DOWN:
				if(m_history_counter >= m_history.size())
					break;
				if(m_history_counter == m_history.size()-1) {
					m_command = m_last_command;
					m_history_counter = m_history.size();
				} else {
					m_command = m_history[++m_history_counter];
				}
				m_command_cursor = m_command.size();
				break;
			case KEY_PPAGE:
				m_cursor = std::max<int>(m_cursor - get_log_height(), 0);
				break;
			case KEY_NPAGE:
				m_cursor = std::min<int>(m_cursor + get_log_height(), std::max<int>(0, m_lines.size() - get_log_height() - 1) );
				break;
			case KEY_EOL:
				m_cursor = m_lines.size() - get_log_height() - 1;
				break;
			case KEY_BACKSPACE:
			case 127:
				if(m_command_cursor > m_command_prefix.size()) {
					m_command_cursor--;
					m_command.erase(m_command_cursor, 1);
				}
				break;
			case KEY_HOME:
				m_command_cursor = m_command_prefix.size();
				break;
			case KEY_END:
				m_command_cursor = m_command.size();
				break;
			case KEY_LEFT:
				if(m_command_cursor > m_command_prefix.size()) {
					m_command_cursor--;
				}
				break;
			case KEY_RIGHT:
				if(m_command_cursor < m_command.size())
					m_command_cursor++;
				break;
			case '\t':
				if(code_complete_handler) {
					std::string cmd = GetCommand();
					std::string complete = code_complete_handler(cmd, cmd.size());
					m_command.insert(m_command_cursor, complete);
					m_command_cursor += complete.size();
				}
				break;
			case KEY_DL:
			case 21:
				m_command = m_command_prefix;
				m_command_cursor = m_command_prefix.size();
				break;
			case KEY_MOUSE:
				break;
			case KEY_RESIZE:
				getmaxyx(stdscr, m_window_height, m_window_width);
				break;
			default:
				if(input == '\n') {
					std::string cmd = GetCommand();
					
					m_history.push_back(m_command);
					m_history_counter = m_history.size();
					
					m_command = m_command_prefix;
					m_command_cursor = m_command_prefix.size();
					dirty = true;
					if(m_auto_refresh) {
						Refresh();
					}
					return cmd;
				} else {
					m_command.insert(m_command_cursor++, 1, input);
				}
				
				if(m_auto_code_complete) {
					std::string cmd = GetCommand();
					code_complete_handler(cmd, cmd.size());
				}
		}
		
		dirty = true;
	}
}
