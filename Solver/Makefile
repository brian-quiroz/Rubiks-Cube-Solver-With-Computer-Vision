RubiksCubeSolver: main.o Piece.o Solver.o PreconditionViolationException.o
	g++ -std=c++11 -g -Wall main.o Piece.o Solver.o PreconditionViolationException.o -o RubiksCubeSolver
	
main.o: main.cpp Solver.h Node.h Node.hpp Queue.h Queue.hpp LinkedList.h LinkedList.hpp PreconditionViolationException.h
	g++ -std=c++11 -g -Wall -c main.cpp

PreconditionViolationException.o: PreconditionViolationException.h PreconditionViolationException.cpp
	g++ -std=c++11 -g -Wall -c PreconditionViolationException.cpp
	
Piece.o: Piece.h Piece.cpp
	g++ -std=c++11 -g -Wall -c Piece.cpp
	
Solver.o: Piece.h Solver.h Solver.cpp Queue.h Queue.hpp
	g++ -std=c++11 -g -Wall -c Solver.cpp

clean:
	rm *.o RubiksCubeSolver