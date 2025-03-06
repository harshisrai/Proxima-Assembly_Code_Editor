#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <map>
using namespace std;

vector<pair<unsigned int, string>> instructions;
map<string,unsigned int> labels;
unsigned int pc = 0x0; // Starting program counter

// Function to check if a line is an instruction (not .data, labels, or empty lines)
int lineType(const string &line) {
    // 1 for instruction, 2 for text labels, 0 for others
    static bool text = false;
    if (line.find(".text") != string::npos) { text = true;cout<<line<<endl;  return 0; }
    if (line.empty() || line[0] == '.') return 0; // Ignore empty lines and directives
    if (line.find(':') != string::npos && text == false) return 0; // Ignore data labels
    if (line.find(':') != string::npos && text == true) {
        //divide in the into two parts - word before: and words afer : and return 2
        size_t pos = line.find(':');
        string label = line.substr(0, pos);
        label.erase(label.find_last_not_of(" ") + 1); // Trim trailing spaces
        // Extract second part (ignoring leading spaces)
        string instruct = line.substr(pos + 1);
        cout<<pc<<endl;
        cout<<"Label: "<<label<<" "<<"Instruction: "<<instruct<<endl;
        // if instruction string is empty
        // labels.push_back({pc, label}); 
        labels[label] = pc;
        if (!instruct.empty()){
            instructions.push_back({pc, instruct});
            stringstream ss(instruct);
            string opcode;
            ss >> opcode;
            if (opcode == "lw" || opcode == "lh" || opcode == "lb" || opcode == "ld") {
                pc += 8;
            } else {
                pc += 4;
            }
        }
        return 2;
    }
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
        
        if (type == 1) {
            instructions.push_back({pc, line});
            
            // Determine instruction size
            stringstream ss(line);
            string opcode;
            ss >> opcode;
            if (opcode == "lw" || opcode == "lh" || opcode == "lb" || opcode == "ld") {
                pc += 8;
            } else {
                pc += 4;
            }
        } 
    }
    
    // Write to output file with program counter
    for (const auto &[pc_value, instr] : instructions) {
        outputFile << "0x" << hex << setw(8) << setfill('0') << pc_value << ": " << instr << endl;
    }
    outputFile << endl;

    //print label with PC
    for (const auto &[label,pc_value] : labels) {
        outputFile << "0x" << hex << setw(8) << setfill('0') << pc_value << ": " << label << endl;
    }



    cout << "Processed assembly instructions saved to refined_output.s" << endl;
    inputFile.close();
    outputFile.close();
    return 0;
}