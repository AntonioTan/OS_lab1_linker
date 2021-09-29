/*
 * @Description: getToken test unit
 * @Version: 1.0
 * @Autor: Tabbit
 * @Date: 2021-09-26 22:13:45
 * @LastEditors: Tabbit
 * @LastEditTime: 2021-09-28 21:28:09
 */
 // #include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
const char* sep = " \t\n";  // seperator for the strtok
int moduleCnt = 1;  // module count should start from 1
int moduleBaseAddress = 0; // the initial module base address, start from 0
// use a int array to store module base address
int moduleArr[3000];
ifstream f;
int symbolNum = 0;
struct Symbol {
    string name;
    int used;
    int address;
    Symbol() :name(), used(), address() {}
    Symbol(string n, int u, int a) {
        name = n;
        used = u;
        address = a;
    }
}symbolTable[1000];
char* createWordArr(string line);
void getNewLine(char** wordArr, char** word, int* lineNum, int* offset);
void getOffSet(char** word, char** wordArr, int* offset);
int readInt(char** wordArr, char** word, int* lineNum, int* lineOffset);
string readSymbol(char** wordArr, char** word, int* lineNum, int* lineOffset);
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
        cout << "After: " << *wordArr << endl;
        char* wordArrCp = createWordArr(line);
        *word = strtok(wordArrCp, sep);
        *offset = 0;
        *lineNum += 1;
    }
    getOffSet(word, wordArr, offset);
}

void getOffSet(char** word, char** wordArr, int* offset) {
    string line = (string)(*wordArr);
    int gap = strcspn(line.substr(*offset, line.length()-*offset).c_str(), *word);
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
    if(iearChar!="I"&&iearChar != "E"&&iearChar!="A"&&iearChar!="R") {
        throw ParseError(2, *lineNum, *lineOffset);
    }
    return iearChar[0];
}


char* createWordArr(string line) {
    char* wordArr = new char[line.length() + 1];
    strcpy(wordArr, line.c_str());
    return wordArr;
}

void createSymbol(string name, int num, int baseAddr) {
    Symbol a = Symbol(name, false, num + baseAddr);
    symbolTable[symbolNum++] = Symbol(name, 0, num + baseAddr);
}

string convertCharPointer(char* target) {
    string rst = "";
    for(int i=0; *(target+i); i++) {
        rst += *(target+i);
    }
    return rst;
}

void Pass1() {
    // initialize variable part
    string line; // string line for the next line read from ifstream
    int lineNum = 0; // line number 
    int offSet = 0; // line offset of each line
    char* word; // the current word
    char* wordArr; // the char* for current line
    while (!f.eof()) {
        // initialize
        moduleArr[moduleCnt] = moduleBaseAddress;
        int defCount = readInt(&wordArr, &word, &lineNum, &offSet);
        // if no defCount left, end the scan 
        if (defCount == -1) {
            break;
        }
        // Now we have def Count, we need to get symbol
        for (int symCnt = 0; symCnt < defCount; symCnt++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
            int symbolNum = readInt(&wordArr, &word, &lineNum, &offSet);
            if (symbolNum == -1) {
                throw ParseError(0, lineNum, offSet);
            }
            createSymbol(symbolName, symbolNum, moduleBaseAddress);
        }
        // Now we move to useCount part
        int useCount = readInt(&wordArr, &word, &lineNum, &offSet);
        if (useCount == -1) {
            throw ParseError(0, lineNum, offSet);
        }
        for(int i=0; i<useCount; i++) {
            string symbolName = readSymbol(&wordArr, &word, &lineNum, &offSet);
        }
        int instCount = readInt(&wordArr, &word, &lineNum, &offSet);
        if (instCount == -1) {
            throw ParseError(0, lineNum, offSet);
        }
        for(int i=0; i<instCount; i++) {
            char addressMode = readIEAR(&wordArr, &word, &lineNum, &offSet);
            cout << offSet << endl;
            int operand = readInt(&wordArr, &word, &lineNum, &offSet);
            cout << offSet << endl;
            if(operand == -1) {
                throw ParseError(0, lineNum, offSet);
            }
        }
        moduleCnt++;
        moduleBaseAddress += instCount;
    }
    // if symbolCnt is still 0, meaning no definition is included should throw an error and abort the process
    if (symbolNum == 0) {
        throw ParseError(0, lineNum, offSet);
    }
    // print out the symbol table
    cout << "Symbol Table" << endl;
    for(int i=0; i<symbolNum; i++) {
        cout << symbolTable[i].name << "=" << symbolTable[i].address << endl;
    }

}


int main(int argc, char* argv[]) {
    try {
        f.open("input");
        // getToken();
        Pass1();
    } catch (ParseError parseError) {
        __parseerror(parseError.getErrcode(), parseError.getLineNum(), parseError.getLineOffset());
    }
}