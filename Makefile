
test_main: test_main.cpp Commands.cpp
	g++ $^ -O2 -g -o commands

test_curses_console: test_curses_console_main.cpp CursesConsole.cpp
	g++ $^ -lcurses -g -o $@
	
test_curses_commands_console: test_curses_console_commands_main.cpp CursesConsole.cpp Commands.cpp
	g++ $^ -lcurses -g -o $@

clean:
	rm -f test_curses_commands_console test_curses_console test_main
