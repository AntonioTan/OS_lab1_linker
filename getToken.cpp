/*
 * @Description: getToken test unit
 * @Version: 1.0
 * @Autor: Tabbit
 * @Date: 2021-09-26 22:13:45
 * @LastEditors: Tabbit
 * @LastEditTime: 2021-09-27 10:40:24
 */
 // #include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
void getToken() {
    ifstream f("input");
    cout << "Reading file...." << endl;
    string line;
    int len = 0;
    int lineNum = 1;
    char *brkt;
    char *word;
    while (!f.eof()) {
        // read a whole line from file stream
        getline(f, line);
        // count line length
        int len = line.length();
        // count offset of a line
        int offSet = 0;
        // convert to char array
        char char_arr[len+1];
        strcpy(char_arr, line.c_str());
        for(word = strtok(char_arr, " \t\n"); word; word=strtok(NULL, " \t")) {
            // find the first place that match
            offSet = strcspn(line.substr(offSet, len).c_str(), word);
            cout << word << "   lineNum:" << lineNum << "    offSet:" << offSet+1 << endl;
        } 
        len += line.length();
        lineNum ++;
    }
}

int main(int argc, char* argv[]) {
    printf("Hello World\n");
    getToken();
}