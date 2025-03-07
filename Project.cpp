#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <string>
#include <cstdint>
using namespace std;

const uint32_t DATA_SEGMENT_START = 0x10000000;

void processDataSegment(const string &inputFileName, const string &outputFileName) {
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName);
    
    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Error opening file!" << endl;
        return;
    }
    
    string line;
    uint32_t currentAddress = DATA_SEGMENT_START;
    bool inDataSection = false;
    
    while (getline(inputFile, line)) {
        istringstream iss(line);
        string word;
        
        if (line.find(".data") != string::npos) {
            inDataSection = true;
            continue;
        }
        
        if (inDataSection) {
            size_t pos;
            if ((pos = line.find(".word")) != string::npos) {
                istringstream values(line.substr(pos + 5));
                int num;
                while (values >> num) {
                    outputFile << "0x" << hex << uppercase << currentAddress << " 0x" 
                               << setw(8) << setfill('0') << num << endl;
                    currentAddress += 4;
                }
            } else if ((pos = line.find(".dword")) != string::npos) {
                istringstream values(line.substr(pos + 6));
                long long num;
                while (values >> num) {
                    outputFile << "0x" << hex << uppercase << currentAddress << " 0x" 
                               << setw(16) << setfill('0') << num << endl;
                    currentAddress += 8;
                }
            } else if ((pos = line.find(".half")) != string::npos) {
                istringstream values(line.substr(pos + 5));
                int num;
                while (values >> num) {
                    outputFile << "0x" << hex << uppercase << currentAddress << " 0x" 
                               << setw(4) << setfill('0') << num << endl;
                    currentAddress += 2;
                }
            } else if ((pos = line.find(".byte")) != string::npos) {
                istringstream values(line.substr(pos + 5));
                int num;
                while (values >> num) {
                    outputFile << "0x" << hex << uppercase << currentAddress << " 0x" 
                               << setw(2) << setfill('0') << num << endl;
                    currentAddress += 1;
                }
            } else if ((pos = line.find(".asciz")) != string::npos) {
                size_t start = line.find('"', pos);
                size_t end = line.rfind('"');
                if (start != string::npos && end != string::npos && start < end) {
                    string str = line.substr(start + 1, end - start - 1);
                    for (char c : str) {
                        outputFile << "0x" << hex << uppercase << currentAddress << " 0x" 
                                   << setw(2) << setfill('0') << (int)c << endl;
                        currentAddress += 1;
                    }
                    currentAddress += 1;
                }
            }
        }
    }
    
    inputFile.close();
    outputFile.close();
    
    cout << "Processing complete. Check " << outputFileName << endl;
}

int main() {
    processDataSegment("assembly.s", "output.txt");
    return 0;
}
