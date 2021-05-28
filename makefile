perft.o: perft.cpp
	echo "Done!"

perft.cpp: MoveUndo.o MoveList.o Board.o doPerft.cpp
	echo "Perft.cpp exists, compiling..."
	g++ doPerft.cpp -std=c++11 -o perft


MoveUndo.o: MoveUndo.h
	echo "Checking if MoveUndo.h exists..."

MoveUndo.h: MoveUndo.cpp
	echo "MoveUndo.h exists."
	echo "Checking if MoveUndo.cpp exists..."

MoveUndo.cpp:
	echo "MoveUndo.cpp exists. Compiling..."
	g++ -c MoveUndo.cpp -std=c++11


MoveList.o: MoveList.cpp
	echo "Checking if MoveList.cpp exists..."

MoveList.cpp:
	echo "MoveList.cpp exists. Compiling..."
	g++ -c MoveList.cpp -std=c++11


Board.o: Board.h
	echo "Checking if Board.h exists..."

Board.h: Board.cpp
	echo "Board.h exists."
	echo "Checking if Board.cpp exists..."

Board.cpp: BoardMoveGen.o MoveUndo.o MoveList.o
	echo "Board.cpp exists."
	echo "Checking if BoardMoveGen.cpp exists..."
	g++ -c Board.cpp -std=c++11 BoardMoveGen.o MoveUndo.o MoveList.o

BoardMoveGen.o: BoardMoveGen.cpp
	echo "Checking if BoardMoveGen.cpp exists."

BoardMoveGen.cpp:
	echo "BoardMoveGen.cpp exists. Compiling..."
	g++ -c BoardMoveGen.cpp -std=c++11


doPerft.cpp:Board.h MoveList.cpp MoveUndo.h
	echo "doPerft.cpp exists. Compiling..."
	g++ -c doPerft.cpp -std=c++11

main: perft.o
	g++ perft.cpp -std=c++11 -o perft



