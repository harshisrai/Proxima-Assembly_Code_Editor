#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

// Function to check if a line is an instruction (not .data, labels, or empty lines)
int lineType(const string &line) {
    // 1 for instruction, 2 for text labels
    //0 for others
    static bool text = false;
    if (line.empty() || line[0] == '.') return 0; // Ignore empty lines and directives
    if(line.find(".text")){text = true; return 0;}
    if (line.find(':') != string::npos && text==false) return 0; // Ignore data labels
    if (line.find(':') != string::npos && text==true) return 2; // Ignore data labels
    return 1;
}

int main() {
    ifstream inputFile("assembly.s"); // Input assembly file
    ofstream outputFile("refined_output.s"); // Output file with PC
    
    if (!inputFile || !outputFile) {
        cerr << "Error opening file!" << endl;
        return 1;
    }
    
    string line;
    vector<string> instructions;
    unsigned int pc = 0x0; // Starting program counter

    while (getline(inputFile, line)) {
        // Remove comments from the line
        size_t commentPos = line.find('#');
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim leading and trailing spaces
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        int type = lineType(line);
        if (1) {
            instructions.push_back(line);
        }
        else if(2){
            // text label of an instruction 
            // get pc and label stored as pair
        }
    }
    
    // Write to output file with program counter
    for (const auto &instr : instructions) {
        outputFile << "0x" << hex << setw(8) << setfill('0') <<pc<<" "<< instr << endl;
        // cout<<instr<<endl;
        stringstream ss(instr);
        string substring;
        ss >> substring;
        if(substring=="lw" || substring=="lh" || substring=="lb" || substring=="ld"){
            cout<<"here"<<endl;
            pc+=8;
        }
        else pc += 4; // Increment PC by 4 for each instruction
    }
    
    cout << "Processed assembly instructions saved to refined_output.s" << endl;
    inputFile.close();
    outputFile.close();
    return 0;
}