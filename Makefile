EXTRA_CCFLAGS   = -Wall -pedantic -ansi
CXXFLAGS        = $(DEBUG_LEVEL) $(EXTRA_CCFLAGS)

ini: ini.o
	g++ -o ini ini.o $(CXXFLAGS)

ini.o: ini.cc doctest.h
	g++ -c ini.cc

clean:
	rm ini.o
