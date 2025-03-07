#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <string>
using namespace std;

vector<pair<unsigned int, string>> instructions_sample;
map<string,unsigned int> labels_sample;
unsigned int pcsample = 0x0; // Starting program counter

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
        cout<<pcsample<<endl;
        cout<<"Label: "<<label<<" "<<"Instruction: "<<instruct<<endl;
        // if instruction string is empty
        // labels.push_back({pcsample, label}); 
        labels_sample[label] = pcsample;
        if (!instruct.empty()){
            instructions_sample.push_back({pcsample, instruct});
            stringstream ss(instruct);
            string opcode;
            ss >> opcode;
            if (opcode == "lw" || opcode == "lh" || opcode == "lb" || opcode == "ld") {
                pcsample += 8;
            } else {
                pcsample += 4;
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
    ifstream inputFileSample("assembly.s");        // Input assembly file
    ofstream outputFileSample("refined_output.asm"); // Output file with PC

    if (!inputFileSample || !outputFileSample)
    {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    string line_sample;
    vector<pair<unsigned int, string>> instructions_sample;
    vector<pair<unsigned int, string>> labels_sample;
    unordered_map<string, long long> labelmap;
    unsigned int pcsample = 0x0; // Starting program counter

    // section to map labels_sample to their addresses in .data section
    getline(inputFileSample, line_sample);
    if (line_sample.find(".data") != string::npos)
    {

        int curraddress = 0x10000000;
        // while loop should keep running until it finds .text
        // modify below line for the same
        while (getline(inputFileSample, line_sample) && line_sample.find(".text") == string::npos)
        {
            // recognize the label that is at the start of the line_sample
            if (line_sample.find(':') != string::npos)
            {
                // divide in the into two parts - word before: and words afer : and return 2
                size_t pos = line_sample.find(':');
                string labelname = line_sample.substr(0, pos);
                labelname.erase(labelname.find_last_not_of(" ") + 1); // Trim trailing spaces
                labelmap[labelname] = curraddress;
                // if word , add 4 for each word in the second part , if byte add 1 , if half , add 2 , if doubleword add 8
                //  Extract second part (ignoring leading spaces)
                string after = line_sample.substr(pos + 1);
                int mul = 1;
                after.erase(after.find_last_not_of(" ") + 1); // Trim trailing spaces
                if (after.find(".word") != string::npos)
                {
                    // count number of words in this line_sample
                    mul = 4;
                }
                else if (after.find(".byte") != string::npos)
                {
                    // count number of bytes in this line_sample
                    mul = 1;
                }
                else if (after.find(".half") != string::npos)
                {
                    // count number of halfs in this line_sample
                    mul = 2;
                }
                else if (after.find(".doubleword") != string::npos)
                {
                    // count number of doublewords in this line_sample
                    mul = 8;
                }
                // count number of words/bytes/halfs/doublewords in this line_sample which are separated by commas or spaces
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
    while (getline(inputFileSample, line_sample))
    {
        // Remove comments from the line_sample
        size_t commentPos = line_sample.find('#');
        if (commentPos != string::npos)
        {
            line_sample = line_sample.substr(0, commentPos);
        }

        // Trim leading and trailing spaces
        line_sample.erase(0, line_sample.find_first_not_of(" \t"));
        line_sample.erase(line_sample.find_last_not_of(" \t") + 1);

        int type = lineType(line_sample);

        if (type == 1)
        {
            stringstream ss(line_sample);
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
                    instructions_sample.push_back({pcsample, "auipc " + reg + " 65536"});
                    pcsample += 4;
                    instructions_sample.push_back({pcsample, opname + " " + reg + " " + to_string(labelmap[label] - 268435456 - (pcsample - 4)) + "(" + reg + ")"});
                    pcsample += 4;
                }
                else
                {
                    instructions_sample.push_back({pcsample, line_sample});
                    pcsample += 4;
                }
            }
            else
            {
                instructions_sample.push_back({pcsample, line_sample});
                pcsample += 4;
            }
        }
        else if (type == 2)
        {
            labels_sample.push_back({pcsample, line_sample});
        }
    }

    // Write to output file with program counter
    for (const auto &[pcsample_value, instr] : instructions_sample)
    {
        outputFileSample << "0x" << hex << setw(8) << setfill('0') << pcsample_value << ": " << instr << endl;
    }
    outputFileSample << endl;

    //print label with pcsample
    for (const auto &[label,pcsample_value] : labels_sample) {
        outputFileSample << "0x" << hex << setw(8) << setfill('0') << pcsample_value << ": " << label << endl;
    }



    cout << "Processed assembly instructions saved to refined_output.s" << endl;
    inputFileSample.close();
    outputFileSample.close();
    return 0;
}