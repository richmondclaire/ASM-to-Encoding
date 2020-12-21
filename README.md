# ASM-to-Encoding
This code accepts a MIPS assembly file as input and outputs the corresponding binary machine encoding. 

- ASM.cpp – Main file that takes command line argument of file to read and iterates
through all instructions in file, printing their encodings.

- ASMParser.* - Reads input file, checking for syntax, translating each MIPS assembly
instruction into its parts and its encoding. Retains list of instructions to iterate over.

- Instruction.* - Each instruction read in is stored in an instance of this class.

- Opcode.* - Stores templates for instruction formats.

- RegisterTable.* - Stores names for all MIPS registers.

- inst.* - Example MIPS input file and its output. 
