SRC = main.cpp graphics_window.cpp utility.cpp rubiksCube.cpp rubiksCubeAi.cpp
OBJ=$(SRC:%.cpp=%.o)

LFLAGS=-lSDL2 -lSDL2_image -lSDL2_ttf ../../gameEngineLib/gameEngine.o 
CURRDIR=`pwd`
CXX=g++ -g -std=c++14  -Wall -o ../build/build.out  -I../../gameEngineLib -I../../gameEngineLib/src

DEBUGF=g++ -g -std=c++14  -Wall -o ../build/build.out

all: $(OBJ)
	$(CXX) $(OBJ) $(LFLAGS)



clean:
	rm *.o	

debug:$(OBJ)
	$(CXX) $(OBJ) $(LFLAGS)