#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <string>
using namespace std;

vector<pair<unsigned int, string>> instructions;
map<string,unsigned int> labels;
unsigned int pc = 0x0; // Starting program counter

// Function to check if a line is an instruction (not .data, labels, or empty lines)
int lineType(const string &line)
{
    // 1 for instruction, 2 for text labels, 0 for others
    static bool text = false;
    if (line.find(".text") != string::npos)
    {
        text = true;
        cout << line << endl;
        return 0;
    }
    if (line.empty() || line[0] == '.')
        return 0; // Ignore empty lines and directives
    if (line.find(':') != string::npos && text == false)
        return 0; // Ignore data labels
    if (line.find(':') != string::npos && text == true)
    {
        // divide in the into two parts - word before: and words afer : and return 2
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
        string after = line.substr(pos + 1);
        cout << "before: " << label << " " << "after: " << after << endl;
        return 2;
    }
    return 1;
}

int main()
{
    ifstream inputFile("assembly.s");        // Input assembly file
    ofstream outputFile("refined_output.s"); // Output file with PC

    if (!inputFile || !outputFile)
    {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    string line;
    vector<pair<unsigned int, string>> instructions;
    vector<pair<unsigned int, string>> labels;
    unordered_map<string, long long> labelmap;
    unsigned int pc = 0x0; // Starting program counter

    // section to map labels to their addresses in .data section
    getline(inputFile, line);
    if (line.find(".data") != string::npos)
    {

        int curraddress = 0x10000000;
        // while loop should keep running until it finds .text
        // modify below line for the same
        while (getline(inputFile, line) && line.find(".text") == string::npos)
        {
            // recognize the label that is at the start of the line
            if (line.find(':') != string::npos)
            {
                // divide in the into two parts - word before: and words afer : and return 2
                size_t pos = line.find(':');
                string labelname = line.substr(0, pos);
                labelname.erase(labelname.find_last_not_of(" ") + 1); // Trim trailing spaces
                labelmap[labelname] = curraddress;
                // if word , add 4 for each word in the second part , if byte add 1 , if half , add 2 , if doubleword add 8
                //  Extract second part (ignoring leading spaces)
                string after = line.substr(pos + 1);
                int mul = 1;
                after.erase(after.find_last_not_of(" ") + 1); // Trim trailing spaces
                if (after.find(".word") != string::npos)
                {
                    // count number of words in this line
                    mul = 4;
                }
                else if (after.find(".byte") != string::npos)
                {
                    // count number of bytes in this line
                    mul = 1;
                }
                else if (after.find(".half") != string::npos)
                {
                    // count number of halfs in this line
                    mul = 2;
                }
                else if (after.find(".doubleword") != string::npos)
                {
                    // count number of doublewords in this line
                    mul = 8;
                }
                // count number of words/bytes/halfs/doublewords in this line which are separated by commas or spaces
                int count = 0;
                for (int i = 0; i < after.size(); i++)
                {
                    if (after[i] == ',' || after[i] == ' ')
                    {
                        count++;
                    }
                }
                curraddress += count * mul;
            }
        }
        for (auto i : labelmap)
        {
            cout << i.first << " " << i.second << endl;
        }
    }
    while (getline(inputFile, line))
    {
        // Remove comments from the line
        size_t commentPos = line.find('#');
        if (commentPos != string::npos)
        {
            line = line.substr(0, commentPos);
        }

        // Trim leading and trailing spaces
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        int type = lineType(line);

        if (type == 1)
        {
            stringstream ss(line);
            string opname;
            ss >> opname;
            if (opname == "lw" || opname == "lh" || opname == "lb" || opname == "ld")
            {
                // extract the register from the line
                string reg;
                ss >> reg;
                // extract the base register from the line
                string label;
                ss >> label;
                bool itislabel;
                if (label.find('(') != string::npos)
                {
                    itislabel = false;
                }
                else
                {
                    itislabel = true;
                }
                if (itislabel)
                {
                    instructions.push_back({pc, "auipc " + reg + " 65536"});
                    pc += 4;
                    instructions.push_back({pc, opname + " " + reg + " " + to_string(labelmap[label] - 268435456 - (pc - 4)) + "(" + reg + ")"});
                    pc += 4;
                }
                else
                {
                    instructions.push_back({pc, line});
                    pc += 4;
                }
            }
            else
            {
                instructions.push_back({pc, line});
                pc += 4;
            }
        }
        else if (type == 2)
        {
            labels.push_back({pc, line});
        }
    }

    // Write to output file with program counter
    for (const auto &[pc_value, instr] : instructions)
    {
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