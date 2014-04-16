

OBJS = main.o sudoku.o

EXENAME = sudoku_solve

CC = g++
CCOPTS = -c -g
LINK = g++
LINKOPTS = -o

all : $(EXENAME)

$(EXENAME) : $(OBJS)
	$(LINK) $(LINKOPTS) $(EXENAME) $(OBJS)

main.o : main.cpp
	$(CC) $(CCOPTS) main.cpp

sudoku.o : sudoku.h sudoku.cpp debug.h
	$(CC) $(CCOPTS) sudoku.cpp

clean :
	rm -f *.o $(EXENAME) 2>/dev/null
