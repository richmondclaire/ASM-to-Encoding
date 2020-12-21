EXECS = ASM
OBJS = ASM.o Opcode.o RegisterTable.o Instruction.o ASMParser.o 

CC = g++
LD = $(CC)
CCFLAGS = -std=c++17 -Wall -Wno-deprecated -Werror=return-type -g -c
LDFLAGS = -Wall -std=c++17 -g

all: $(EXECS)

ASM: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o ASM

ASM.o: ASM.cpp ASMParser.h ASMParser.cpp Instruction.cpp
	$(CC) $(CCFLAGS) ASM.cpp

ASMParser.o: ASMParser.cpp ASMParser.h
	$(CC) $(CCFLAGS) ASMParser.cpp

Opcode.o: Opcode.cpp Opcode.h
	$(CC) $(CCFLAGS) Opcode.cpp

RegisterTable.o: RegisterTable.cpp RegisterTable.h
	$(CC) $(CCFLAGS) RegisterTable.cpp

Instruction.o: Instruction.cpp Instruction.h Opcode.h RegisterTable.h
	$(CC) $(CCFLAGS) Instruction.cpp

clean:
	/bin/rm -f $(OBJS) $(EXECS)