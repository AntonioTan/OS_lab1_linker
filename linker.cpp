/*
 * @Description: getToken test unit
 * @Version: 1.0
 * @Autor: Tabbit
 * @Date: 2021-09-26 22:13:45
 * @LastEditors: Tabbit
 * @LastEditTime: 2021-09-29 00:52:34
 */
 // #include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
const char* sep = " \t\n";  // seperator for the strtok
// use a int array to store module base address
ifstream f;
int symbolNum = 0;
struct Symbol {
    string name;
    int used; // 0 unused 1 used
    int address;
    int multi;
    int module;
    Symbol() :name(), used(), address(), multi(), module() {}
    Symbol(string n, int u, int a, int m, int mod) {
        name = n;
        used = u;
        address = a;
        multi = m;
        module = mod;
    }
}symbolTable[1000];
char* createWordArr(string line);
void getNewLine(char** wordArr, char** word, int* lineNum, int* offset);
void getOffSet(char** word, char** wordArr, int* offset);
int readInt(char** wordArr, char** word, int* lineNum, int* lineOffset);
string readSymbol(char** wordArr, char** word, int* lineNum, int* lineOffset);
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
    string err3 = "Error:" + def + " is not defined; zero used";
    string errstr[7] = {
        "Error: Absolute address exceeds machine size; zero used",
        "Error: Relative address exceeds module size; zero used",
        "Error: External address exceeds length of uselist; treated as immediate",
        err3,
        "Error: This variable is multiple times defined; first value used",
        "Error: Illegal immediate value; treated as 9999" "Error: Illegal opcode; treated as 9999"
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
    string line;
    if (!f.eof()) {
        getline(f, line);
    }
    else {
        return;
    }
    *wordArr = createWordArr(line);
    char* wordArrCp = createWordArr(line);
    *word = strtok(wordArrCp, sep);
    *lineNum += 1;
    *offset = 0;
    while (!*word) {
        if (!f.eof()) {
            getline(f, line);
        }
        else {
            return;
        }
        *wordArr = createWordArr(line);
        char* wordArrCp = createWordArr(line);
        *word = strtok(wordArrCp, sep);
        *offset = 0;
        *lineNum += 1;
    }
    getOffSet(word, wordArr, offset);
}

void getOffSet(char** word, char** wordArr, int* offset) {
    string line = (string)(*wordArr);
    int gap = strcspn(line.substr(*offset, line.length() - *offset).c_str(), *word);
    *offset += gap;
    // cout << "offset: " << *offset << "  len: " << line.length()-*offset << "    line: " << *wordArr <<endl;
    // cout << "line: " << line.substr(*offset, line.length()-*offset) << "    word: " << *word << "    offset: "<< *offset << endl;
}

int readInt(char** wordArr, char** word, int* lineNum, int* lineOffset) {
    *word = strtok(NULL, sep);
    while (!*word) {
        if (!f.eof()) {
            getNewLine(wordArr, word, lineNum, lineOffset);
        }
        else {
            return -1;
        }
    }
    getOffSet(word, wordArr, lineOffset);
    string wordCp = (string)*word;
    for (int i = 0; i < wordCp.length(); i++) {
        if (!isdigit(wordCp[i])) {
            throw ParseError(0, *lineNum, *lineOffset);
        }
    }
    int rst = stoi(wordCp);
    if (rst >= pow(2, 30)) {
        // if __parseerror is reached the return statement is unreachable since the process will be aborted
        throw ParseError(0, *lineNum, *lineOffset);
    }
    return rst;
}

string readSymbol(char** wordArr, char** word, int* lineNum, int* lineOffset) {
    // get the name of this symbol
    *word = strtok(NULL, sep);
    while (!*word) {
        if (!f.eof()) {
            getNewLine(wordArr, word, lineNum, lineOffset);
        }
        else {
            throw ParseError(1, *lineNum, *lineOffset);
        }
    }
    getOffSet(word, wordArr, lineOffset);
    string name = (string)*word;
    for (int i = 0; i < name.length(); i++) {
        if (!isalnum(name[i])) {
            throw ParseError(1, *lineNum, *lineOffset);
        }
    }
    return name;
}

char readIEAR(char** wordArr, char** word, int* lineNum, int* lineOffset) {
    *word = strtok(NULL, sep);
    while (!*word) {
        if (!f.eof()) {
            getNewLine(wordArr, word, lineNum, lineOffset);
        }
        else {
            throw ParseError(2, *lineNum, *lineOffset);
        }
    }
    getOffSet(word, wordArr, lineOffset);
    string iearChar = (string)*word;
    if (iearChar != "I" && iearChar != "E" && iearChar != "A" && iearChar != "R") {
        throw ParseError(2, *lineNum, *lineOffset);
    }
    return iearChar[0];
}


char* createWordArr(string line) {
    char* wordArr = new char[line.length() + 1];
    strcpy(wordArr, line.c_str());
    return wordArr;
}

int checkSymbolExist(string name) {
    for (int i = 0; i < symbolNum; i++) {
        if (symbolTable[i].name == name) {
            return i;
        }
    }
    return -1;
}

void createSymbol(string name, int num, int baseAddr, int module) {
    Symbol a = Symbol(name, 0, num + baseAddr, 0, module);
    int pastSymbolIndex = checkSymbolExist(name);
    if (pastSymbolIndex == -1) {
        symbolTable[symbolNum++] = a;
    }
    else {
        symbolTable[pastSymbolIndex].multi = 1;
    }
}

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
    int offSet = 0; // line offset of each line
    char* word; // the current word
    char* wordArr; // the char* for current line
    while (!f.eof()) {
        // initialize
        int defCount = readInt(&wordArr, &word, &lineNum, &offSet);
        // if no defCount left, end the scan 
        if (defCount == -1) {
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
            if (symbolNum == -1) {
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
        if (useCount == -1) {
            throw ParseError(0, lineNum, offSet);
        }
        if (useCount > 16) {
            throw ParseError(5, lineNum, offSet);
        }
        for (int i = 0; i < useCount; i++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
        }
        int instCount = readInt(&wordArr, &word, &lineNum, &offSet);
        if (instCount == -1) {
            throw ParseError(0, lineNum, offSet);
        }
        for (int i = 0; i < instCount; i++) {
            char addressMode = readIEAR(&wordArr, &word, &lineNum, &offSet);
            int operand = readInt(&wordArr, &word, &lineNum, &offSet);
            if (operand == -1) {
                throw ParseError(0, lineNum, offSet);
            }
        }
        // check warning Rule5 
        for (int i = 0; i < defCount; i++) {
            if (tempSymNum[i] > instCount) {
                tempSymNum[i] = 0;
                printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", moduleCnt, tempSymName[i].c_str(), tempSymNum[i], instCount);
            }
            createSymbol(tempSymName[i], tempSymNum[i], moduleBaseAddress, moduleCnt);
        }
        moduleCnt++;
        moduleBaseAddress += instCount;
        if (moduleBaseAddress > 512) {
            throw ParseError(6, lineNum, offSet);
        }
    }
    // if symbolCnt is still 0, meaning no definition is included should throw an error and abort the process
    if (symbolNum == 0) {
        throw ParseError(0, lineNum, offSet);
    }

}
string getAddr(int addr) {
    if (addr > 99) {
        return to_string(addr);
    }
    else {
        string rst = "";
        int bit = 0;
        while (addr != 0) {
            int mod = addr % 10;
            rst = to_string(mod) + rst;
            addr = (addr - mod) / 10;
            bit++;
        }
        while (bit < 3) {
            rst = "0" + rst;
            bit++;
        }
        return rst;
    }
}

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
    int offSet = 0; // line offset of each line
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
        if (defCount == -1) {
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
            string addrStr = getAddr(addr);
            if (opcode > 9 && addressMode != 'I') {
                operand = 9999;
                cout << addrStr << ": " << to_string(operand) << " " << __printerror(6) << endl;  // Rule 11
                continue;
            }
            if (addressMode == 'A') {
                // Rule 8
                if (operandNum > 512) {
                    string substitute = to_string(opcode*1000);
                    cout << addrStr << ": " << substitute.c_str() << " " << __printerror(0) << endl;
                }
                else {
                    cout << addrStr << ": " << to_string(operand) << endl;
                }
            }
            else if (addressMode == 'R') {
                if (operandNum > instCount) {
                    string substitute = to_string(moduleBaseAddress);
                    cout << addrStr.c_str() << ": " << substitute.c_str() << " " << __printerror(1) << endl;
                }
                else {
                    cout << addrStr << ": " << to_string(operand + moduleBaseAddress) << endl;
                }
            }
            else if (addressMode == 'E') {
                if (operandNum > useCount - 1) {
                    cout << addrStr << ": " << to_string(operand) << " " << __printerror(2) << endl;
                    continue;
                }
                useState[operandNum] = 1;
                string symbol = symNameL[operandNum];
                int globalAddr = getGlobalAddr(symbol);
                if (globalAddr == -1) {
                    string substitute = to_string(opcode*1000);
                    cout << addrStr << ": " << substitute.c_str() << " " << __printerror(3, symbol) << endl;
                }
                else {
                    cout << addrStr << ": " << to_string(opcode * 1000 + globalAddr) << endl;
                }
            }
            else {
                if (opcode > 9) {
                    operand = 9999;
                    cout << addrStr << ": " << to_string(operand) << " " << __printerror(6) << endl;  // Rule 11
                }
                else {
                    cout << addrStr << ": " << to_string(operand) << endl;
                }
            }

        }
        moduleCnt++;
        moduleBaseAddress += instCount;
        // print use state Warning rule7
        for(int i=0; i<useCount; i++) {
            if(useState[i]==0) {
                printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", moduleCnt, symNameL[i].c_str());
            }
        }
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
    for(int i=0; i<symbolNum; i++) {
        if(symbolTable[i].used==0) {
            // rule 4
            printf("Warning: Module %d: %s was defined but never used\n", symbolTable[i].module, symbolTable[i].name.c_str());
        }
    }

}

int main(int argc, char* argv[]) {
    try {
        string fileAddr = "lab1_assign/"+(string)argv[1];
        f.open(fileAddr);
        // getToken();
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