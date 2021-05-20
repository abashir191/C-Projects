/*
    SIC/XE Disassembler
    Abdullahi Mohamed - 821652137
    Abdullahi Mohamed - 822489428  
*/

#include "disassembler.hpp"

int main(int argc, char *argv[])
{

    if (argc == 3)     // the number of arguments on command line
    {                  
        obj = argv[1]; // object file is the first argument
        sym = argv[2]; // symbol file is the second argument
    }
    else
    {
        cout << "Please enter the missing file(s) needed" << endl; // incorrect number of arguments
        exit(1);
    }

    string line; //string of each line of the object file

    //open both object and listing files
    objectCodeFile.open(obj);
    assemblyListFile.open(lis);

    if (!objectCodeFile.is_open())
    {
        cout << "error: unable to open file " << obj << "\n"; // file cannot open
        exit(1);
    }
    else
    {
        while (getline(objectCodeFile, line))
        {
            char first = line[0];
            if (first == 'H')
            {
                headerRecord(line); //use header record function
            }
            else if (first == 'T')
            {
                textRecord(line); //use text record function
            }
            else if (first == 'E')
            {
                endRecord(line); //use end text function
            }
            else
            {
                continue;
            }
        }
    }

    objectCodeFile.close();
    assemblyListFile.close();
    return 0;
}

string removeWhiteSpace(string &s)
{
    const char* whitespaces = "\n\r\t\f\v";
    size_t first = s.find_first_not_of(whitespaces);
    size_t last = s.find_last_not_of(whitespaces);
    return first == last ? string() : s.substr(first, last - first + 1);
}

int hexToDecimal(int x)
{
    stringstream stream;
    stream << x;
    stream >> hex >> x;

    return x;
}

string decimalToHex (int x) {
    stringstream stream;
    stream << hex << x;
    addrStr = (stream.str());

    return addrStr;
}

void headerRecord(string line)
{

    //print starting address to the assembly list file
    addrStr = "";
    for (int i = 9; i <= 12; i++)
    {
        addrStr += line[i];
    }

    assemblyListFile << addrStr << "\t";

    //converts starting address to a decimal number 
    addrInt = hexToDecimal(stoi(addrStr));

    // parse the name of the program from object code file
    string progName = line.substr(1, 6);

    assemblyListFile << progName << "\t\t\tSTART\t";
    assemblyListFile << addrInt << "\n";

    // parse the starting address from object code file 
    progStartAddr = line.substr(7, 13);

    // parse the ending address from object code file
    progEndAddr = line.substr(14, 19);
}

void addressingTypes(string line, int index)
{
    // this function allows us to know if the addressing type is either immediate, indirect, or simple
    char immediate[4] = {'1', '5', '9', 'D'};
    char indirect[4] = {'2', '6', 'A', 'E'};
 
    for (int i = 0; i < 4; i++)
    {
        if (line[index + 1] == immediate[i])
        {
            assemblyListFile << "\t\t#";
            break;
        }
        else if (line[index + 1] == indirect[i])
        {
            assemblyListFile << "\t\t@";
            break;
        }
        else
        {
            assemblyListFile << "\t\t";
            break;
        }
    }
}

void registerForClear(string line, int index)
{
    // this function helps us get the register for format 2 instructions 
    // (in our case CLEAR is the only format 2 instruction we need to look out for)
    char registers[9] = {'0', '1', '2', '3', '4', '5', '6', '8', '9'};
    string mnemonics[9] = {"A", "X", "L", "B", "S", "T", "F", "PC", "SW"};

    for (int i = 0; i < 7; i++)
    {
        if (line[index+2] == registers[i])
        {
            assemblyListFile << mnemonics[i];
        }
    }
}

void checkLTORG(string line, int index, string currAddr)
{
    // this function checks for literals found in the symbol table file
    // prints 'LTORG' to the assembly list file upon finding a literal
    string symbolLine;
    string currLiteralLine;
    if (!symbolTableFile.is_open())
    {
        cout << "Unable to open the symbol file!" << sym << "\n";
        exit(1);
    }
    else
    {
        while (getline(symbolTableFile, symbolLine))
        {
            if (symbolLine[8] == '=')
            {
                for (int i = 21; i < symbolLine.length(); i++)
                {
                    if (!isspace(symbolLine[i]))
                    {
                        currLiteralLine = currLiteralLine + symbolLine[i];
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            if (currLiteralLine != currAddr)
            {
                continue;
            }
            else {
                assemblyListFile << "\t\t\t\tLTORG\n";
                break;
            }
        }
    }
}

void reserve(string lastAddr)
{
    // this function takes care of the 'RESW' labels that are found in the symbol table
    // prints them to the assembly list file
    string symbolLine;
    int previousAddr;
    int flag;
    symbolTableFile.open(sym);
    if (symbolTableFile.is_open())
    {
        while (getline(symbolTableFile, symbolLine))
        {
            string labelAddr;
            string label;
            int labelInt;
            
            if (symbolLine[8] == 'V' || symbolLine[8] == '-') {
                continue;
            }
               
            else if (symbolLine[8] == 'L' || symbolLine == "\0"){
                break;
            }
                
            else
            {
                int cursor = 0; 
                while (cursor < 6) {
                    label = label + symbolLine[cursor];
                    cursor++;
                }
                cursor = 10;
                while(cursor < 14) {
                    labelAddr = labelAddr + symbolLine[cursor];
                    cursor++;
                }

                labelInt = stoi(labelAddr, nullptr, 16);
            }
        
            if (labelInt >= stoi(lastAddr, nullptr, 16)) {
                break;
            }
    
            if (flag == 1 && labelInt >= addrInt)
            {
                assemblyListFile << (labelInt - previousAddr) / 3 << "\n";
                addrInt = labelInt;
                decimalToHex(addrInt);
            }
            
            if (labelInt >= addrInt)
            {
                assemblyListFile << labelAddr << "\t" << label << "\tRESW\t";
                previousAddr = labelInt;
                flag = 1; 
            }
        }
    }
    symbolTableFile.close();

    assemblyListFile << (hexToDecimal(stoi(lastAddr)) - addrInt) / 3 << "\n"; 
}

string capitalizeLoc (string& s) {
    // this function capitalizes the letters in our addresses
    for (int i = 0; i < 4; i++)
    {
        if (isalpha(s[i])) {
            s[i] = toupper(s[i]);
        }                        
    }

    return s;
}

void labelChecker(string targetAddr)
{
    // this function checks the labels found in the symbol table file
    // it prints them in the assembly list file in their correct positions
    symbolTableFile.open(sym);
    if (symbolTableFile.is_open())
    {
        string symbolLine;
        while (getline(symbolTableFile, symbolLine))
        {
            string currLabelLine;
            string currAddrLine;
            string currLiteralLine;
            
            if (symbolLine[1] != '=' && symbolLine[8] != '=')
            {
                for (int i = 0; i < 6; i++)
                {
                    currLabelLine = currLabelLine + symbolLine[i];
                    currAddrLine = currAddrLine + symbolLine[i+8];
                }
            }

            else if (symbolLine[1] == '=' || symbolLine[8] == '=')
            {
                for (int i = 21; i < symbolLine.length(); i++)
                {
                    if (!isspace(symbolLine[i])) {
                        currLiteralLine += symbolLine[i];
                        
                    }              
                    else {
                        continue;
                    }      
                }
            }

            if (currAddrLine == targetAddr)
            {
                currLabel = currLabelLine;
                removeWhiteSpace(currLabel);

                assemblyListFile << currLabel; //print label
                break;
            }
    
            else if (currLiteralLine == targetAddr)
            {
                currLiteralLine = "";
                if (symbolLine[1] == '=')
                {
                    int cursor = 1;
                    while (cursor <= 10 || isspace(symbolLine[cursor])) {
                        currLiteralLine = currLiteralLine + symbolLine[cursor];
                        cursor++;
                    }
                }
                else if (symbolLine[8] == '=')
                {
                    int cursor = 8;
                    while (cursor <= 17 || isspace(symbolLine[cursor])) {
                        currLiteralLine = currLiteralLine + symbolLine[cursor];
                        cursor++;
                    }   
                }
                removeWhiteSpace(currLiteralLine);    
                
                assemblyListFile << currLiteralLine; 
                break;
            }
        }
    }
    symbolTableFile.close();
}

void textRecord(string line)
{
    string textAddr = line.substr(1, 6);

    string currA = "00";
    currA.append(addrStr);

    // function call which will allow the program to find and print all the RESW labels in the program
    // only does this if the current address and text records beginning address are not equal
 
    if (currA != textAddr)
    {
        reserve(textAddr);
    }

    // starting address of the text record becomes the new current address 
    textAddr = "";
    for (int i = 3; i < 7; i++)
    {
        textAddr += line[i];
    }

    addrStr = textAddr;
    addrInt = hexToDecimal(stoi(addrStr));

    //starting position of first object code in text record
    int index = 9;

    // string for the length of the text record 
    string textRecordLengthStr = "";
    textRecordLengthStr.append(1, line[7]).append(1, line[8]);

    // converting the text record length which is of type string to type int
    int textRecordLength = (stoi(textRecordLengthStr, nullptr, 16)) * 2;

    // loop that runs all the way to the end of the current text record
    while (index < textRecordLength + 9)
    {
        // print current address of text record and prepending 0's to make size of address 4 #'s
        string currAddr;
        if (addrStr.length() == 1)
        {
            addrStr.insert(0, "000"); 
        }
        else if (addrStr.length() == 2)
        {
            addrStr.insert(0, "00");
        }
        else if (addrStr.length() == 3)
        {
            addrStr.insert(0, "0");
        }
        currAddr.insert(0, "00" + addrStr);

        symbolTableFile.open(sym);

        //function call that prints LTORG if literal was found
        checkLTORG(line, index, currAddr);

        assemblyListFile << addrStr << "\t"; 
        symbolTableFile.close();

        symbolTableFile.open(sym);
        bool literal = false;   // checks if literal is there
        bool label = false;     // checks if label is there
        int literalAdd = 0;     // variable for keeping track of the length of the literal

        //checks if address is either a label or a literal
        if (!symbolTableFile.is_open())
        {
            cout << "error: unable to open file " << sym << "\n";
            exit(1);
        }
        else {
            string symbolLine;
            while (getline(symbolTableFile, symbolLine))
            {
                string currLabelLine;
                string currLiteralLine;

                // conditional statement that gets the address of the label 
                if (symbolLine[1] != '=' && symbolLine[8] != '=')
                {
                    for (int i = 8; i < 14; i++)
                    {
                        currLabelLine = currLabelLine + symbolLine[i];
                    }
                }
                // conditional statement that gets the address of the literal 
                if (symbolLine[1] == '=' || symbolLine[8] == '=')
                {
                    for (int i = 21; i < symbolLine.length(); i++)
                    {
                        if (!isspace(symbolLine[i]))
                        {
                            currLiteralLine = currLiteralLine + symbolLine[i]; // spaces are disregarded
                        }
                        else
                        {
                            continue;      
                        }
                    }
                }

                // conditional statement that checks if the current address in the symbol table file matches the label
                if (currLabelLine == currAddr)
                {
                    currLabelLine = "";
                    label = true;
                    currLabelLine = symbolLine.substr(0,6); 
                    assemblyListFile << currLabelLine;
                    break;
                }
                // conditional statement that checks if the current address in the symbol table file matches the literal 
                else if (currLiteralLine == currAddr)
                {
                    currLiteralLine = "";
                    string literalLength;

                    // gets literal name and prints it to the assembly list file
                    if (symbolLine[1] == '=')
                    {
                        literalLength = symbolLine.substr(12,15);
                        currLiteralLine = symbolLine.substr(1,12);
                    }
                    else if (symbolLine[8] == '=')
                    {
                        literalLength = symbolLine.substr(19,22);

                        for (int i = 8; i < 18; i++)
                        {
                            currLiteralLine += symbolLine[i];
                        }
                    }
                    
                    literalAdd = stoi(literalLength, nullptr, 10);
                    addrInt += (stoi(literalLength, nullptr, 10)) / 2;
                    literal = true;

                    //print literal line to the assembly list file
                    assemblyListFile << "\t\t*" << "\t\t" << currLiteralLine << "\t";
                    for (int i = 3; currLiteralLine[i] != '\''; i++)
                    {
                        assemblyListFile << currLiteralLine[i];
                    }
                    break;
                }
            }
            assemblyListFile << "\t\t"; 
        }
        
        symbolTableFile.close();

        string opcodeNumber;
        opcodeNumber = line[index];

        // removes the two rightmost bits for the opcode
        if (line[index + 1] == '1' || line[index + 1] == '2' || line[index + 1] == '3') {
            opcodeNumber.append("0");
        }
            
        else if (line[index + 1] == '5' || line[index + 1] == '6' || line[index + 1] == '7') {
            opcodeNumber.append("4");
        }
            
        else if (line[index + 1] == '9' || line[index + 1] == 'A' || line[index + 1] == 'B') {
            opcodeNumber.append("8");
        }
            
        else if (line[index + 1] == 'D' || line[index + 1] == 'E' || line[index + 1] == 'F') {
            opcodeNumber.append("C");
        }

        else {
            opcodeNumber += line[index + 1];
        }
            

        //only when it is not a literal
        if (!literal)
        {
            //iterate through the sicInstrcutionSet table and check for an opcode that matches
            for (int i = 0; i < sizeof(table) / sizeof(*table); i++)
            {
                if (table[i].opcode == opcodeNumber)
                {
                    int addExtraByte = 0; //adds two half bytes to index if format 4
                    int x = 0;  //X flag for index addressing mode

                    // conditional statement to check if it is a format 4
                    if (line[index + 2] == '1' || line[index + 2] == '9')
                    {
                        assemblyListFile << "+";
                        addExtraByte = 2;
                    }
                    // calculating the displacement or address from object code;
                    string currStr;
                    for (int i = 0; i < 3 + addExtraByte; i++)
                    {
                        currStr = currStr + line[index + i + 3];
                    }
                    // change Base register -- opcode of 68 is the LDB mnemonic
                    if (opcodeNumber == "68")
                    {
                        baseReg = currStr;
                    }
                    // change X register
                    else if (opcodeNumber == "04")
                    {
                        indexReg = currStr;
                    }

                    int programCounter = addrInt + addExtraByte + table[i].format; // find the program index of the curent instruction 
                    assemblyListFile << table[i].mnemonic; // print the corresponding mnemonic

                    // function call that determines if instruction is indirect, immediate, or simpleand prints @, #, or just leaves a blank.
                    addressingTypes(line, index);

                    //print register for mnemonic CLEAR
                    char registers[9] = {'0', '1', '2', '3', '4', '5', '6', '8', '9'};
                    string mnemonics[9] = {"A", "X", "L", "B", "S", "T", "F", "PC", "SW"};

                    if(opcodeNumber == "B4") {
                        registerForClear(line, index);
                    }
                    else if (opcodeNumber == "4C")
                    {
                        //skips printing to RSUB instruction -- displacement for RSUB is simply 0's
                    }
                    // calculates and prints the displacement constant
                    else if (line[index + 2] == registers[0])
                    {
                        int out = hexToDecimal(stoi(currStr));
                        assemblyListFile << out;
                    }
                    // print the label from address
                    else if (line[index + 2] == registers[1])
                    {
                        string targetAddr = "0";

                        if (currStr.length() == 3) {
                            targetAddr.append("00" + currStr);
                        }
                            
                        else if (currStr.length() == 4) {
                            targetAddr.append("0" + currStr);
                        }
                            
                        else {
                            targetAddr.append(currStr);
                        }
                            
                        labelChecker(targetAddr);
                    }

                    // calculates and prints label from address (PC) + disp
                    else if (line[index + 2] == '2')
                    {
                        int addr;
                        if (currStr[0] == 'F')
                        {
                            string curr = "F" + currStr;
                            int disp = -1 * (65535 - stoi(curr, nullptr, 16) + 1);
                            addr = (disp) + programCounter;
                        }
                        else {
                            addr = stoi(currStr, nullptr, 16) + programCounter;
                        }    

                        decimalToHex(addr); // function call for converting to hex
                        string printer = addrStr;
                        capitalizeLoc(printer); // function call for capitalizing letters in address 

                        string targetAddr = "00"; 

                        // conditional statement that appends necessary amount of 0's
                        if (printer.length() == 1) {
                            targetAddr.append("000" + printer); 
                        }   

                        else if (printer.length() == 2) {
                            targetAddr.append("00" + printer);
                        }  

                        else if (printer.length() == 3) {
                            targetAddr.append("0" + printer);
                        } 

                        else {
                            targetAddr.append(printer);
                        }
                            
                        labelChecker(targetAddr);
                    }
                    // calculates and prints label from address (B) + disp 
                    else if (line[index + 2] == '4')
                    {
                        int addr;
                        addr = stoi(currStr, nullptr, 16) + stoi(baseReg, nullptr, 16);
                        decimalToHex(addr); // function call for converting to hex
                        string printer = addrStr;
                        capitalizeLoc(printer); // function call for capitalizing letters in address

                        string targetAddr = "00";

                        // conditional statement that appends necessary amount of 0's
                        if (printer.length() == 1)
                        {
                            targetAddr.append("000" + printer);
                        }
                        else if (printer.length() == 2)
                        {
                            targetAddr.append("00" + printer);
                        }
                        else if (printer.length() == 3)
                        {
                            targetAddr.append("0" + printer);
                        }
                        else
                        {
                            targetAddr.append(printer);
                        }

                        labelChecker(targetAddr);
                    }
                    // calculations with index addresssing mode (X)
                    else if (line[index + 2] == registers[7] || line[index + 2] == registers[8] ||
                             line[index + 2] == mnemonics[0].at(0) || line[index + 2] == mnemonics[2].at(0))
                    {
                        x = 1;
                        int address;

                        //calculating the displacement with index addressing mode disp + (X)
                        if (currStr[0] == mnemonics[6].at(0))
                        {
                            string curr = mnemonics[6] + currStr;
                            int disp = -1 * (65535 - stoi(curr, nullptr, 16) + 1);
                            address = (disp) + stoi(indexReg, nullptr, 16);
                        }
                        else
                            address = stoi(currStr, nullptr, 16) + stoi(indexReg, nullptr, 16);

                        // calculates for PC relative with index addressing mode --- (PC) + disp + (X)
                        if (line[index + 2] == mnemonics[0].at(0))
                        {
                            if (currStr[0] == mnemonics[6].at(0))
                            {
                                string curr = mnemonics[6] + currStr;
                                int disp = -1 * (65535 - stoi(curr, nullptr, 16) + 1);
                                address = (disp) + programCounter;
                            }
                            else {
                                address = address + programCounter;
                            }
                                
                        }
                        // calculates for Base relative with index addressing mode --- (B) + disp + (X)
                        else if (line[index + 2] == mnemonics[2].at(0)){
                            address += stoi(baseReg, nullptr, 16);
                        }
                        // calculates and prints displacement constants
                        else if (line[index + 2] == registers[7]) {
                            assemblyListFile << address;
                        }
                            
                        
                        if (line[index + 2] != registers[7])
                        {
                            decimalToHex(address); // conversion to hex
                            string printer = addrStr;
                            capitalizeLoc(printer); // capitalizing letters in address 

                            string targetAddr = "00";

                            // appending necessary amount of 0's
                            if (printer.length() == 1)
                            {
                                targetAddr.append("000" + printer);
                            }
                            else if (printer.length() == 2)
                            {
                                targetAddr.append("00" + printer);
                            }
                            else if (printer.length() == 3)
                            {
                                targetAddr.append("0" + printer);
                            }
                            else
                            {
                                targetAddr.append(printer);
                            }
                            
                            labelChecker(targetAddr); // prints a label if there is a label
                        }
                        assemblyListFile << ",X"; //print ,X after label or constant to signify index addressing mode in assembly list file
                    }

                    // printout all the opcodes 
                    string currRecord;
                    int incrementCounter = (table[i].format * 2) + addExtraByte;
                    int counter = 0;
                    while (counter < incrementCounter) {
                        currRecord = currRecord + line[index+counter];
                        counter++;
                    }
                    if (currLabel.length() != 6 || x != 1) {
                        assemblyListFile << "\t";
                    }
                    assemblyListFile << "\t" << currRecord;

                    //print base and its label when mnemonic is LDB
                    if (opcodeNumber == "68") {
                        assemblyListFile << "\n\t\t\t\tBASE\t" << currLabel;
                    }

                    index = index + incrementCounter; // update & increment index of text record
                    addrInt = addrInt + (table[i].format) + (addExtraByte / 2); // update & increment current address
                    break;
                }
            }
        }
        else {
            index = index + literalAdd; //add literal length to index
        }
            
        decimalToHex(addrInt); // conversion to hex
        assemblyListFile << "\n";

        capitalizeLoc(addrStr); //capitalize all letters in address 
    }
}

void endRecord(string line)
{
    if (stoi(progEndAddr, nullptr, 16) > addrInt) {
        reserve(progEndAddr); //prints ending label RESW to the assembly list file
    }   
    //prints end record with the beginning label 'FIRST'
    assemblyListFile << "\t\t\t\tEND\t\t" << "FIRST";
    labelChecker(progEndAddr);
    assemblyListFile << "\n";
}
