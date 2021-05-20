/*
    SIC/XE Disassembler
    Abdullahi Mohamed - 821652137
    Abdullahi Mohamed - 822489428  
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstddef>
#include <vector>
#include <algorithm>
using namespace std;

void headerRecord(string line);
void reserve(string lastAddr);
string capitalizeLoc (string &s);
void labelChecker(string endAddr);
void textRecord(string line);
void endRecord(string line);
string removeWhiteSpace(string &s); 
int hexToDecimal(int x);
string decimalToHex(int x); 
void addressingTypes (string line, int index);
void registerForClear (string line, int index);
void checkLTORG(string line, int index, string currAdr); 

ifstream objectCodeFile;
ifstream symbolTableFile;
ofstream assemblyListFile;

// keep track of our current address
string addrStr;
int addrInt;

// for the 'FIRST'
string startLabel;

//base register saved as string for base directive
string baseReg;

//x register saved as string for index addressing mode
string indexReg;

string currLabel;

// beginning and ending addresses of file
string progStartAddr;
string progEndAddr;

struct sicInstructionSet {
    string opcode;
    string mnemonic;
    int format;
};


const struct sicInstructionSet table[] = {
    {"18", "ADD", 3},      {"58", "ADDF", 3},     {"40", "AND", 3},
    {"B4", "CLEAR", 2},    {"28", "COMP", 3},     {"88", "COMPF", 3},
    {"24", "DIV", 3},      {"64", "DIVF", 3,},    {"3C", "J", 3},         
    {"30", "JEQ", 3},      {"34", "JGT", 3},      {"38", "JLT", 3},       
    {"48", "JSUB", 3,},    {"00", "LDA", 3},      {"68", "LDB", 3},       
    {"50", "LDCH", 3},     {"70", "LDF", 3},      {"08", "LDL", 3},
    {"6C", "LDS", 3},      {"74", "LDT", 3},      {"04", "LDX", 3},       
    {"D0", "LPS", 3},      {"20", "MUL", 3},      {"60", "MULF", 3},
    {"44", "OR", 3},       {"D8", "RD", 3},       {"4C", "RSUB", 3},
    {"EC", "SSK", 3},      {"0C","STA", 3},       {"78", "STB", 3},       
    {"54", "STCH", 3},     {"80", "STF", 3},      {"D4", "STI", 3},       
    {"14", "STL", 3},      {"7C", "STS", 3},      {"E8", "STSW", 3},
    {"84", "STT", 3},      {"10", "STX", 3},      {"1C", "SUB", 3},       
    {"5C", "SUBF", 3},     {"E0", "TD", 3},       {"2C", "TIX", 3},     {"DC", "WD", 3}
};

// the default names for the object code file, symbol table file, and assembly list file
string obj;
string sym;
string lis = "out.lst";
