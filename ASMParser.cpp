#include "ASMParser.h"

using namespace std;

ASMParser::ASMParser(string filename)
  // Specify a text file containing MIPS assembly instructions. Function
  // checks syntactic correctness of file and creates a list of Instructions.
{
  Instruction i;
  myFormatCorrect = true;

  myLabelAddress = 0x400000;

  labelCounter = 0;

  ifstream in;
  in.open(filename.c_str());
  if(in.bad()){
    myFormatCorrect = false;
  }
  else{
    string line;
    while( getline(in, line)){
      string opcode("");
      string operand[80];
      int operand_count = 0;

      line = getLabel(line);
      getTokens(line, opcode, operand, operand_count);

      if(opcode.length() == 0 && operand_count != 0){
	// No opcode but operands
	myFormatCorrect = false;
	break;
      }

      Opcode o = opcodes.getOpcode(opcode);      
      if(o == UNDEFINED){
	// invalid opcode specified
	myFormatCorrect = false;
	break;
      }

      bool success = getOperands(i, o, operand, operand_count);
      if(!success){
	myFormatCorrect = false;
	break;
      }

      string encoding = encode(i);
      i.setEncoding(encoding);

      myInstructions.push_back(i);
      myLabelAddress = myLabelAddress + 4;
    }
  }

  myIndex = 0;
}

string ASMParser::getLabel(string line){
  string::size_type idx = line.find(':');
  if (idx != string::npos){
    labelTable[labelCounter][0] = line.substr(0, idx);
    labelTable[labelCounter][1] = to_string(myLabelAddress);
    labelCounter++;
    line = line.substr(idx+1, line.length());
  }
  return line;
}

Instruction ASMParser::getNextInstruction()
  // Iterator that returns the next Instruction in the list of Instructions.
{
  if(myIndex < (int)(myInstructions.size())){
    myIndex++;
    return myInstructions[myIndex-1];
  }
  
  Instruction i;
  return i;

}

void ASMParser::getTokens(string line,
			       string &opcode,
			       string *operand,
			       int &numOperands)
  // Decomposes a line of assembly code into strings for the opcode field and operands, 
  // checking for syntax errors and counting the number of operands.
{
    // locate the start of a comment
    string::size_type idx = line.find('#');
    if (idx != string::npos) // found a ';'
	line = line.substr(0,idx);
    int len = line.length();
    opcode = "";
    numOperands = 0;

    if (len == 0) return;
    int p = 0; // position in line

    // line.at(p) is whitespace or p >= len
    while (p < len && isWhitespace(line.at(p)))
	p++;
    // opcode starts
    while (p < len && !isWhitespace(line.at(p)))
    {
	opcode = opcode + line.at(p);
	p++;
    }
    //    for(int i = 0; i < 3; i++){
    int i = 0;
    while(p < len){
      while ( p < len && isWhitespace(line.at(p)))
	p++;

      // operand may start
      bool flag = false;
      while (p < len && !isWhitespace(line.at(p)))
	{
	  if(line.at(p) != ','){
	    operand[i] = operand[i] + line.at(p);
	    flag = true;
	    p++;
	  }
	  else{
	    p++;
	    break;
	  }
	}
      if(flag == true){
	numOperands++;
      }
      i++;
    }

    
    idx = operand[numOperands-1].find('(');
    string::size_type idx2 = operand[numOperands-1].find(')');
    
    if (idx == string::npos || idx2 == string::npos ||
	((idx2 - idx) < 2 )){ // no () found
    }
    else{ // split string
      string offset = operand[numOperands-1].substr(0,idx);
      string regStr = operand[numOperands-1].substr(idx+1, idx2-idx-1);
      
      operand[numOperands-1] = offset;
      operand[numOperands] = regStr;
      numOperands++;
    }
    
    

    // ignore anything after the whitespace after the operand
    // We could do a further look and generate an error message
    // but we'll save that for later.
    return;
}

bool ASMParser::isNumberString(string s)
  // Returns true if s represents a valid decimal integer
{
    int len = s.length();
    if (len == 0) return false;
    if ((isSign(s.at(0)) && len > 1) || isDigit(s.at(0)))
    {
	// check remaining characters
	for (int i=1; i < len; i++)
	{
	    if (!isdigit(s.at(i))) return false;
	}
	return true;
    }
    return false;
}


int ASMParser::cvtNumString2Number(string s)
  // Converts a string to an integer.  Assumes s is something like "-231" and produces -231
{
    if (!isNumberString(s))
    {
	cerr << "Non-numberic string passed to cvtNumString2Number"
		  << endl;
	return 0;
    }
    int k = 1;
    int val = 0;
    for (int i = s.length()-1; i>0; i--)
    {
	char c = s.at(i);
	val = val + k*((int)(c - '0'));
	k = k*10;
    }
    if (isSign(s.at(0)))
    {
	if (s.at(0) == '-') val = -1*val;
    }
    else
    {
	val = val + k*((int)(s.at(0) - '0'));
    }
    return val;
}
		

bool ASMParser::getOperands(Instruction &i, Opcode o, 
			    string *operand, int operand_count)
  // Given an Opcode, a string representing the operands, and the number of operands, 
  // breaks operands apart and stores fields into Instruction.
{

  if(operand_count != opcodes.numOperands(o))
    return false;

  int rs, rt, rd, imm;
  imm = 0;
  rs = rt = rd = NumRegisters;

  int rs_p = opcodes.RSposition(o);
  int rt_p = opcodes.RTposition(o);
  int rd_p = opcodes.RDposition(o);
  int imm_p = opcodes.IMMposition(o);

  if(rs_p != -1){
    rs = registers.getNum(operand[rs_p]);
    if(rs == NumRegisters)
      return false;
  }

  if(rt_p != -1){
    rt = registers.getNum(operand[rt_p]);
    if(rt == NumRegisters)
      return false;

  }
  
  if(rd_p != -1){
    rd = registers.getNum(operand[rd_p]);
    if(rd == NumRegisters)
      return false;

  }

  if(imm_p != -1){
    if(isNumberString(operand[imm_p])){  // does it have a numeric immediate field?
      imm = cvtNumString2Number(operand[imm_p]);
      if(((abs(imm) & 0xFFFF0000)<<1))  // too big a number to fit
	return false;
    }
    else if(operand[imm_p][1] == 'x' || operand[imm_p][1] == 'X'){
      imm = stoi(operand[imm_p].c_str(), nullptr, 16);
    }
    else{ 
      if(opcodes.isIMMLabel(o)){  // Can the operand be a label?
	// Assign the immediate field an address
        for(int i = 0; i < labelCounter; i++){
          if (operand[imm_p].compare(labelTable[i][0]) == 0){ 
            string test = labelTable[i][1];
            imm = stoi(test.c_str(), nullptr, 10);
          }
        }
      }
      else  // There is an error
	return false;
    }

  }

  i.setValues(o, rs, rt, rd, imm);

  return true;
}


string ASMParser::encode(Instruction i)
  // Given a valid instruction, returns a string representing the 32 bit MIPS binary encoding
  // of that instruction.
{
  // Your code here
  Opcode myO = i.getOpcode();
  string s = "";
  if (opcodes.getInstType(myO) == RTYPE) {
    s = encodeR(i);}

  else if (opcodes.getInstType(myO) == ITYPE) {
    s = encodeI(i);}

  else {
    s = encodeJ(i);}

  i.setEncoding(s);
  return s;
}

string ASMParser::dectobin(int dec, int num) //CONVERT FROM DECIMAL TO BINARY
{
  int binaryNum[num];

  for(int i = 0; i<num; i++) {
    binaryNum[i] = 0;
  }

  int j = 0;
  while (dec > 0) {
    binaryNum[j] = dec % 2;
    dec = dec/2;
    j++;
  }

  string s("");

  for(int i=num-1; i>=0; i--) {
    s.append(to_string(binaryNum[i]));
  }
  return s;
}

string ASMParser::encodeR(Instruction i) // ENCODING FOR R INSTRUCTION
{
  Opcode myO = i.getOpcode();
  string s("");
  s.append("000000");
  s.append(dectobin(i.getRS(), 5));
  s.append(dectobin(i.getRT(), 5));
  s.append(dectobin(i.getRD(), 5));
  s.append(dectobin(i.getImmediate(), 5));
  s.append(opcodes.getFunctField(myO));
  return s;
}

string ASMParser::encodeI(Instruction i) // ENCODING FOR I INSTRUCTION
{
  Opcode myO = i.getOpcode();
  string s("");
  s.append(opcodes.getOpcodeField(myO));
  s.append(dectobin(i.getRS(), 5));
  s.append(dectobin(i.getRT(), 5));
  s.append(dectobin(i.getImmediate(), 16));
  return s;
}

string ASMParser::encodeJ(Instruction i) // ENCODING FOR J INSTRUCTION
{
  Opcode myO = i.getOpcode();
  string s("");
  s.append(opcodes.getOpcodeField(myO));
  string label(dectobin(i.getImmediate(), 32)); //0x400000
  string imm("");
  for (int j=4; j<30; j++)  {
    char c = label[j];
    imm.push_back(c);
  }
  s.append(imm);
  return s;
}
