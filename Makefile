TARGETS=player ringmaster

all: $(TARGETS)
clean:
	rm -f $(TARGETS) *.o *~

player: player.cpp potato.hpp
	g++ -g -o $@ $<

ringmaster: ringmaster.cpp potato.hpp
	g++ -g -o $@ $<