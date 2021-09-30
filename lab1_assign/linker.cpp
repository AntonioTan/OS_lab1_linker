/*
 * @Description: linker for OS lab1
 * @Version: 1.0
 * @Autor: Tabbit
 * @Date: 2021-09-26 22:13:45
 * @LastEditors: Tabbit
 * @LastEditTime: 2021-09-30 12:16:47
 */
 // #include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
using namespace std;
const char* sep = " \t\n";  // seperator for the strtok
ifstream f;
int symbolNum = 0;
struct Symbol {
    string name;
    int used; // 0 unused 1 used
    int address;
    int multi;
    int module;
    int relativeNum;
    Symbol() :name(), used(), address(), multi(), module(), relativeNum() {}
    Symbol(string n, int u, int a, int m, int mod, int r) {
        name = n;
        used = u;
        address = a;
        multi = m;
        module = mod;
        r = relativeNum;
    }
}symbolTable[1000];
void __parseerror(int errcode, int lineNum, int lineOffSet);
char* createWordArr(string line);
void getNewLine(char** wordArr, char** word, int* lineNum, int* offset);
void getOffSet(char** word, char** wordArr, int* offset);
int readInt(char** wordArr, char** word, int* lineNum, int* lineOffset);
string readSymbol(char** wordArr, char** word, int* lineNum, int* lineOffset);
char readIEAR(char** wordArr, char** word, int* lineNum, int* lineOffset);
char* createWordArr(string line);
int checkSymbolExist(string name);
string convertCharPointer(char* target);
void printSymbolTable();
void printDefWarning();
void Pass1();
void Pass2();

class ParseError {
public:
    ParseError(int eCode, int lNum, int lOffset) {
        errcode = eCode;
        lineNum = lNum;
        lineOffset = lOffset;
    }
    int getErrcode() {
        return errcode;
    }
    int getLineNum() {
        return lineNum;
    }
    int getLineOffset() {
        return lineOffset;
    }

private:
    int errcode;
    int lineNum;
    int lineOffset;
};

void getToken() {
    cout << "Reading file...." << endl;
    string line;
    int lineNum = 1;
    char* brkt;
    char* word;
    while (!f.eof()) {
        // read a whole line from file stream
        getline(f, line);
        // count line length
        int len = line.length();
        // count offset of a line
        int offSet = 0;
        // convert to char array
        char char_arr[len + 1];
        cout << len << endl;
        strcpy(char_arr, line.c_str());
        word = strtok(char_arr, " \t\n");
        if (!word) {
            cout << "empty line" << endl;
        }
        for (; word; word = strtok(NULL, " \t\n")) {

            // find the first place that match
            offSet = strcspn(line.substr(offSet, len).c_str(), word);
            offSet += ((string)word).length();
        }
        len += line.length();
        lineNum++;
    }
}

string __printerror(int errcode, string def = "") {
    string err3 = "Error: " + def + " is not defined; zero used";
    string errstr[7] = {
        "Error: Absolute address exceeds machine size; zero used",
        "Error: Relative address exceeds module size; zero used",
        "Error: External address exceeds length of uselist; treated as immediate",
        err3,
        "Error: This variable is multiple times defined; first value used",
        "Error: Illegal immediate value; treated as 9999",
        "Error: Illegal opcode; treated as 9999"
    };
    return errstr[errcode];
}

void __parseerror(int errcode, int lineNum, int lineOffSet) {
    string errstr[7] = {
        "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
        "SYM_EXPECTED", // Symbol Expected 
        "ADDR_EXPECTED", // Addressing Expected which is A/E/I/R 
        "SYM_TOO_LONG", // Symbol Name is too long 
        "TOO_MANY_DEF_IN_MODULE", // > 16
        "TOO_MANY_USE_IN_MODULE", // > 16
        "TOO_MANY_INSTR",  // total num_instr exceeds memory size (512)
    };
    printf("Parse Error line %d offset %d: %s\n", lineNum, lineOffSet, errstr[errcode].c_str());
}

void getNewLine(char** wordArr, char** word, int* lineNum, int* offset) {
    // add the length of last word to offset if last word exists
    if (*word) *offset += ((string)*word).length();
    // if line still exists keep reading the next word
    *word = strtok(NULL, sep);
    int addLine = 0;
    // keep last lineNum and last offset
    int lastLineNum = *lineNum;
    int lastOffset = *offset;
    while (!*word) {
        string line;
        if (!f.eof()) {
            getline(f, line);
            *offset = 1;
            *lineNum += 1;
        }
        else {
            *lineNum -= 1;
            if (*lineNum == lastLineNum) {
                // lineNum doesn't change means the next line is null then replace the offset with last one
                *offset = lastOffset;
            }
            return;
        }
        // strtok will mess the original char* so use the copy one
        *wordArr = createWordArr(line);
        char* wordArrCp = createWordArr(line);
        *word = strtok(wordArrCp, sep);
    }
    getOffSet(word, wordArr, offset);
}

void getOffSet(char** word, char** wordArr, int* offset) {
    string line = (string)(*wordArr);
    int gap = strcspn(line.substr(*offset - 1, line.length() - *offset + 1).c_str(), *word);
    *offset += gap;
}

int readInt(char** wordArr, char** word, int* lineNum, int* lineOffset) {
    getNewLine(wordArr, word, lineNum, lineOffset);
    // if next int doesn't exist it can be in two cases
    // case 1 the file is empty no more defCount to read
    // case 2 the next int doesn't exist we need to throw an error
    if (!*word) {
        return -10000;
    }
    string wordCp = (string)*word;
    for (int i = 0; i < wordCp.length(); i++) {
        if (!isdigit(wordCp[i])) {
            throw ParseError(0, *lineNum, *lineOffset);
        }
    }
    int rst = stoi(wordCp);
    if (rst >= pow(2, 30)) {
        throw ParseError(0, *lineNum, *lineOffset);
    }
    return rst;
}

string readSymbol(char** wordArr, char** word, int* lineNum, int* lineOffset) {
    getNewLine(wordArr, word, lineNum, lineOffset);
    if (!*word) {
        throw ParseError(1, *lineNum, *lineOffset);
    }
    string name = (string)*word;
    if (!isalpha(name[0])) {
        throw ParseError(1, *lineNum, *lineOffset);
    }
    for (int i = 1; i < name.length(); i++) {
        if (!isalnum(name[i])) {
            throw ParseError(1, *lineNum, *lineOffset);
        }
    }
    return name;
}

char readIEAR(char** wordArr, char** word, int* lineNum, int* lineOffset) {
    getNewLine(wordArr, word, lineNum, lineOffset);
    if (!*word) {
        throw ParseError(2, *lineNum, *lineOffset);
    }
    string iearChar = (string)*word;
    if (iearChar != "I" && iearChar != "E" && iearChar != "A" && iearChar != "R") {
        throw ParseError(2, *lineNum, *lineOffset);
    }
    return iearChar[0];
}

// copy line to a brand new char*
char* createWordArr(string line) {
    char* wordArr = new char[line.length() + 1];
    strcpy(wordArr, line.c_str());
    return wordArr;
}

// check whether the symbol exists before if it exists return the its index in symbolTable otherwise return -1
int checkSymbolExist(string name) {
    for (int i = 0; i < symbolNum; i++) {
        if (symbolTable[i].name == name) {
            return i;
        }
    }
    return -1;
}

void createSymbol(string name, int num, int baseAddr, int module) {
    Symbol a = Symbol(name, 0, num + baseAddr, 0, module, num);
    symbolTable[symbolNum++] = a;
}

// read string of a char*
string convertCharPointer(char* target) {
    string rst = "";
    for (int i = 0; *(target + i); i++) {
        rst += *(target + i);
    }
    return rst;
}

void Pass1() {
    // initialize variable part
    int moduleCnt = 1;  // module count should start from 1
    int moduleBaseAddress = 0; // the initial module base address, start from 0
    string line; // string line for the next line read from ifstream
    int lineNum = 0; // line number 
    int offSet = 1; // line offset of each line
    char* word; // the current word
    char* wordArr; // the char* for current line
    while (!f.eof()) {
        // initialize
        int defCount = readInt(&wordArr, &word, &lineNum, &offSet);
        // if no defCount left, end the scan 
        if (defCount == -10000) {
            break;
        }
        if (defCount > 16) {
            throw ParseError(4, lineNum, offSet);
        }
        // Now we have def Count, we need to get symbol
        int tempSymNum[defCount];
        string tempSymName[defCount];
        for (int symCnt = 0; symCnt < defCount; symCnt++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
            int symbolNum = readInt(&wordArr, &word, &lineNum, &offSet);
            if (symbolNum == -10000) {
                throw ParseError(0, lineNum, offSet);
            }
            if (symbolName.length() > 16) {
                throw ParseError(3, lineNum, offSet);
            }
            tempSymName[symCnt] = symbolName;
            tempSymNum[symCnt] = symbolNum;
        }
        // Now we move to useCount part
        int useCount = readInt(&wordArr, &word, &lineNum, &offSet);
        if (useCount == -10000) {
            throw ParseError(0, lineNum, offSet);
        }
        if (useCount > 16) {
            throw ParseError(5, lineNum, offSet);
        }
        for (int i = 0; i < useCount; i++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
        }
        int instCount = readInt(&wordArr, &word, &lineNum, &offSet);
        if (moduleBaseAddress + instCount > 512) {
            throw ParseError(6, lineNum, offSet);
        }
        if (instCount == -10000) {
            throw ParseError(0, lineNum, offSet);
        }
        for (int i = 0; i < instCount; i++) {
            char addressMode = readIEAR(&wordArr, &word, &lineNum, &offSet);
            int operand = readInt(&wordArr, &word, &lineNum, &offSet);
            if (operand == -10000) {
                throw ParseError(0, lineNum, offSet);
            }
        }
        // check warning Rule5 
        for (int i = 0; i < defCount; i++) {
            int pastSymbolIndex = checkSymbolExist(tempSymName[i]);
            if (pastSymbolIndex == -1) {
                if (tempSymNum[i] > instCount - 1) {
                    printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", moduleCnt, tempSymName[i].c_str(), tempSymNum[i], instCount - 1);
                    tempSymNum[i] = 0;
                }
                createSymbol(tempSymName[i], tempSymNum[i], moduleBaseAddress, moduleCnt);
            }
            else {
                // even the symbol exists before, still need to check rule 5 according to new moduleBaseAddress
                if (symbolTable[pastSymbolIndex].address - moduleBaseAddress > instCount - 1) {
                    printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", moduleCnt, symbolTable[pastSymbolIndex].name.c_str(), symbolTable[pastSymbolIndex].address-moduleBaseAddress, instCount-1);
                    tempSymNum[i] = 0;
                }
                symbolTable[pastSymbolIndex].multi = 1;
            }
        }
        moduleCnt++;
        moduleBaseAddress += instCount;
    }
    // if symbolCnt is still 0, meaning no definition is included should throw an error and abort the process
    if (moduleCnt == 1) {
        throw ParseError(0, lineNum, offSet);
    }

}

// return string with a length given by bitMax if its original length less than bitMax use 0 to make up
string getAddr(int addr, int bitMax) {
    string rst = "";
    int bit = 0;
    while (addr != 0) {
        int mod = addr % 10;
        rst = to_string(mod) + rst;
        addr = (addr - mod) / 10;
        bit++;
    }
    while (bit < bitMax) {
        rst = "0" + rst;
        bit++;
    }
    return rst;
}

// get global address of a symbol after pass1 and make its used equal to 1 if found one
int getGlobalAddr(string name) {
    for (int i = 0; i < symbolNum; i++) {
        string symName = symbolTable[i].name;
        int symNum = symbolTable[i].address;
        if (symName == name) {
            symbolTable[i].used = 1;
            return symNum;
        }
    }
    return -1;
}


void Pass2() {
    // initialize variable part
    int moduleCnt = 1;  // module count should start from 1
    int moduleBaseAddress = 0; // the initial module base address, start from 0
    string line; // string line for the next line read from ifstream
    int lineNum = 0; // line number 
    int offSet = 1; // line offset of each line
    char* word; // the current word
    char* wordArr; // the char* for current line
    int addr = 0;
    printf("\nMemory Map\n");
    while (!f.eof()) {
        // initialize
        int defCount = readInt(&wordArr, &word, &lineNum, &offSet);
        // Now we have def Count, we need to get symbol
        for (int symCnt = 0; symCnt < defCount; symCnt++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
            int symbolNum = readInt(&wordArr, &word, &lineNum, &offSet);
        }
        if (defCount == -10000) {
            break;
        }
        // Now we move to useCount part
        int useCount = readInt(&wordArr, &word, &lineNum, &offSet);
        string symNameL[useCount];
        int useState[useCount];
        for (int i = 0; i < useCount; i++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
            symNameL[i] = symbolName;
            useState[i] = 0;
        }
        int instCount = readInt(&wordArr, &word, &lineNum, &offSet);
        for (int i = 0; i < instCount; i++, addr++) {
            char addressMode = readIEAR(&wordArr, &word, &lineNum, &offSet);
            int operand = readInt(&wordArr, &word, &lineNum, &offSet);
            int operandNum = operand % 1000;
            int opcode = operand / 1000;
            string addrStr = getAddr(addr, 3);
            string opeAddr = getAddr(operand, 4);
            if (opcode > 9 && addressMode != 'I') {
                operand = 9999;
                cout << addrStr << ": " << getAddr(operand, 4) << " " << __printerror(6) << endl;  // Rule 11
                continue;
            }
            if (addressMode == 'A') {
                // Rule 8
                if (operandNum > 511) {
                    string substitute = getAddr(opcode * 1000, 4);
                    cout << addrStr << ": " << substitute.c_str() << " " << __printerror(0) << endl;
                }
                else {
                    cout << addrStr << ": " << opeAddr << endl;
                }
            }
            else if (addressMode == 'R') {
                // Rule 9
                if (operandNum > instCount) {
                    string substitute = getAddr(opcode * 1000 + moduleBaseAddress, 4);
                    cout << addrStr.c_str() << ": " << substitute.c_str() << " " << __printerror(1) << endl;
                }
                else {
                    cout << addrStr << ": " << getAddr(operand + moduleBaseAddress, 4) << endl;
                }
            }
            else if (addressMode == 'E') {
                // Rule 6
                if (operandNum > useCount - 1) {
                    cout << addrStr << ": " << opeAddr << " " << __printerror(2) << endl;
                    continue;
                }
                useState[operandNum] = 1;
                string symbol = symNameL[operandNum];
                int globalAddr = getGlobalAddr(symbol);
                if (globalAddr == -1) {
                    // Rule 3
                    string substitute = getAddr(opcode * 1000, 4);
                    cout << addrStr << ": " << substitute.c_str() << " " << __printerror(3, symbol) << endl;
                }
                else {
                    cout << addrStr << ": " << getAddr(opcode * 1000 + globalAddr, 4) << endl;
                }
            }
            else {
                // Rule 10
                if (opcode > 9) {
                    operand = 9999;
                    cout << addrStr << ": " << to_string(operand) << " " << __printerror(5) << endl;  // Rule 11
                }
                else {
                    cout << addrStr << ": " << opeAddr << endl;
                }
            }

        }
        // print use state Warning rule7
        for (int i = 0; i < useCount; i++) {
            if (useState[i] == 0) {
                printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", moduleCnt, symNameL[i].c_str());
            }
        }
        moduleCnt++;
        moduleBaseAddress += instCount;
    }

}



void printSymbolTable() {
    // print out the symbol table
    cout << "Symbol Table" << endl;
    for (int i = 0; i < symbolNum; i++) {
        if (symbolTable[i].multi == 0) {
            cout << symbolTable[i].name << "=" << symbolTable[i].address << endl;
        }
        else {
            // Rule 2
            cout << symbolTable[i].name << "=" << symbolTable[i].address << " " << __printerror(4) << endl;
        }
    }
}

void printDefWarning() {
    cout << endl;
    for (int i = 0; i < symbolNum; i++) {
        if (symbolTable[i].used == 0) {
            // Rule 4
            printf("Warning: Module %d: %s was defined but never used\n", symbolTable[i].module, symbolTable[i].name.c_str());
        }
    }

}

int main(int argc, char* argv[]) {
    try {
        string fileAddr = (string)argv[1];
        f.open(fileAddr);
        Pass1();
        f.close();
        printSymbolTable();
        f.open(fileAddr);
        Pass2();
        f.close();
        printDefWarning();
        return 0;
    }
    catch (ParseError parseError) {
        __parseerror(parseError.getErrcode(), parseError.getLineNum(), parseError.getLineOffset());
        return 0;
    }
}