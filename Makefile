all:
	g++  commands.cpp main.cpp -O2 -g -o commands
bind:
	g++  commands.cpp bind.cpp main.cpp -O0 -g -o commands
