#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstdint>
#include <iomanip>
#include <unordered_map>
#include <fstream>
#include <cstdint>
#include <thread>
#include <chrono>
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

struct InstructionInfo {
    string type;
    uint8_t opcode;
    uint8_t funct3;
    uint8_t funct7;

};
string pc;
map<string, int> labels={
    {"branch1",0x58},
    {"branch2",0x70},
    {"branch3",0x74},
    {"branch4",0x78}
};
map<string, InstructionInfo> instructionMap = {
    //"name" {"type","opcode","funct3",funct7"}
    {"add",  {"R", 0x33, 0x0, 0x00}},
    {"sub",  {"R", 0x33, 0x0, 0x20}},
    {"sll",  {"R", 0x33, 0x1, 0x00}}, 
    {"slt",  {"R", 0x33, 0x2, 0x00}}, 
    {"sltu", {"R", 0x33, 0x3, 0x00}}, 
    {"xor",  {"R", 0x33, 0x4, 0x00}}, 
    {"srl",  {"R", 0x33, 0x5, 0x00}}, 
    {"sra",  {"R", 0x33, 0x5, 0x20}}, 
    {"or",   {"R", 0x33, 0x6, 0x00}}, 
    {"and",  {"R", 0x33, 0x7, 0x00}},
    {"mul",  {"R", 0x33, 0x0, 0x01}},
    {"div",  {"R", 0x33, 0x4, 0x01}},
    {"rem",  {"R", 0x33, 0x6, 0x01}},

    {"addi", {"I", 0x13, 0x0, 0x00}},
    {"addi", {"I", 0x13, 0x0, 0x00}}, 
    {"andi", {"I", 0x13, 0x7, 0x00}}, 
    {"ori",  {"I", 0x13, 0x6, 0x00}},

    {"lw",   {"I", 0x03, 0x2, 0x00}},

    {"sw",   {"S", 0x23, 0x2, 0x00}},
    {"sb",   {"S", 0x23, 0x0, 0x00}},
    {"sd",   {"S", 0x23, 0x3, 0x00}},
    {"sh",   {"S", 0x23, 0x1, 0x00}},

    {"beq",  {"SB", 0x63, 0x0, 0x00}},
    {"bne",  {"SB", 0x63, 0x1, 0x00}},
    {"bge",  {"SB", 0x63, 0x5, 0x00}},
    {"blt",  {"SB", 0x63, 0x4, 0x00}},
    {"lui",  {"U", 0x37, 0x0, 0x00}},
    {"auipc",  {"U", 0x17, 0x0, 0x00}},
    {"jal",  {"UJ", 0x6F, 0x0, 0x00}},
    {"lb",{"I",0x03,0x0,0x00}},
    {"lh",{"I",0x03,0x1,0x00}},
    {"ld",{"I",0x03,0x3,0x00}},
    
    
};

vector<string> tokenize(const string& line) {
    vector<string> tokens;
    string token;
    istringstream iss(line); // Convert the string `line` into a stream object
    while (iss >> token) { // Extract tokens from the stream (separated by spaces)
        if (token[0] == '#') break;
        size_t pos;
        while ((pos = token.find(',')) != string::npos) { // Find ',' in token
            token.erase(pos, 1);  // Remove ',' from the token
        }
        tokens.push_back(token);
    }
    return tokens;
}

uint8_t parseRegister(const string& reg) {
    if (reg.empty() || reg[0] != 'x') {
        throw invalid_argument("Invalid register: " + reg);
    }
    int num;
    try {
        num = stoi(reg.substr(1));
    } catch (...) {
        throw invalid_argument("Invalid register: " + reg);
    }
    if (num < 0 || num > 31) {
        throw invalid_argument("Register out of range: " + reg);
    }
    return static_cast<uint8_t>(num);
}

int32_t parseImmediate(const string& immStr, string type) {
    size_t pos = 0;
    int32_t imm;
    try {
        if (immStr.size() > 2 && immStr.substr(0, 2) == "0x") {
            imm = stoi(immStr, &pos, 16);
        } 
        else if(type=="UJ" || type=="SB"){
            cout<<"type: "<<type<<endl;
            int num = stoi(pc, 0, 16);
            imm = labels[immStr]-num;
            cout<<dec<<imm<<endl;
            cout<<hex<<imm<<endl;
            return imm;
        }
        else {
            imm = stoi(immStr, &pos);
        }
    } catch (...) {
        throw invalid_argument("Invalid immediate: " + immStr);
    }
    if (pos != immStr.size()) {
        throw invalid_argument("Invalid immediate: " + immStr);
    }
    return imm;
}

int main() {
    ifstream inputFileSample("assembly.s");        // Input assembly file
    ofstream outputFileSample("input_pc.asm"); // Output file with PC

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



    cout << "Processed assembly instructions saved to input_pc.asm" << endl;
    inputFileSample.close();
    outputFileSample.close();

    //main.cpp starts here

    std::this_thread::sleep_for(std::chrono::seconds(1));


    fstream input_file,output_file;
    input_file.open("input_pc.asm",ios::in);

    if (input_file.is_open()) { 
        string line;
        output_file.open("output.mc", ios::out); 
        // Read data from the file object and put it into a string.
        output_file<<"Machine Code, Assembly Code"<<endl;
    while (getline(input_file, line)) { 
    // cout<<line<<endl;
    auto tokens = tokenize(line);
    if (tokens.empty()) {
        cerr << "Error: Empty input" << endl;
        return 1;
    }
    pc = tokens[0];
    string op = tokens[1];
    if (instructionMap.find(op) == instructionMap.end()) {
        cerr << "Error: Unknown instruction '" << op << "'" << endl;
        return 1;
    }

    const auto& info = instructionMap[op];
    try {
        uint32_t machine_code = 0;
        if (info.type == "R") {
            if (tokens.size() != 5) {
                throw invalid_argument("R-type expects 3 operands");
            }
            uint8_t rd = parseRegister(tokens[2]);
            uint8_t rs1 = parseRegister(tokens[3]);
            uint8_t rs2 = parseRegister(tokens[4]);
            machine_code = (info.funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (info.funct3 << 12) | (rd << 7) | info.opcode;
        } 
        else if (info.type == "I") {
            /* 
            for load instructions, if the immediate is label: then source address reg and destination registers are same
            else take the register specfied in the instruction with the offset 
            */
            if (tokens.size() != 5) {
                throw invalid_argument("I-type expects 3 operands");
            }
            uint8_t rd = parseRegister(tokens[2]);
            uint8_t rs1 = parseRegister(tokens[3]);
            int32_t imm = parseImmediate(tokens[4],"I");
            if (imm < -2048 || imm > 2047) {
                throw invalid_argument("I-type immediate out of range");
            }
            uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFF;
            machine_code = (imm12 << 20) | (rs1 << 15) | (info.funct3 << 12) | (rd << 7) | info.opcode;
        } 
        else if (info.type == "S") {
            if (tokens.size() != 4) {
                throw invalid_argument("S-type expects 3 operands");
            }
            uint8_t rs2 = parseRegister(tokens[2]);
            string mem = tokens[3];
            size_t lparen = mem.find('(');
            size_t rparen = mem.find(')');
            if (lparen == string::npos || rparen == string::npos || lparen >= rparen) {
                throw invalid_argument("Invalid memory operand format");
            }
            int32_t imm = parseImmediate(mem.substr(0, lparen),"S");
            uint8_t rs1 = parseRegister(mem.substr(lparen + 1, rparen - lparen - 1));
            if (imm < -2048 || imm > 2047) {
                throw invalid_argument("S-type immediate out of range");
            }
            uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFF;
            uint32_t imm_high = (imm12 >> 5) & 0x7F;
            uint32_t imm_low = imm12 & 0x1F;
            machine_code = (imm_high << 25) | (rs2 << 20) | (rs1 << 15) | (info.funct3 << 12) | (imm_low << 7) | info.opcode;
        } 
        else if (info.type == "SB") {
            if (tokens.size() != 5) {
                throw invalid_argument("SB-type expects 3 operands");
            }
            uint8_t rs1 = parseRegister(tokens[2]);
            uint8_t rs2 = parseRegister(tokens[3]);
            int32_t imm = parseImmediate(tokens[4],"SB");
            if (imm % 2 != 0) {
                throw invalid_argument("SB offset must be even");
            }
            int32_t shifted_imm = imm;
            if (shifted_imm < -4096 || shifted_imm > 4095) {
                throw invalid_argument("SB offset out of range");
            }
            uint32_t imm_enc = static_cast<uint32_t>(shifted_imm);
            uint32_t imm12 = (imm_enc >> 11) & 0x1;
            uint32_t imm10_5 = (imm_enc >> 5) & 0x3F;
            uint32_t imm4_1 = (imm_enc >> 1) & 0xF;
            uint32_t imm11 = (imm_enc >> 10) & 0x1;
            uint32_t imm_31_25 = (imm12 << 6) | imm10_5;
            uint32_t imm_11_7 = (imm4_1 << 1) | imm11;
            machine_code = (imm_31_25 << 25) | (rs2 << 20) | (rs1 << 15) | (info.funct3 << 12) | (imm_11_7 << 7) | info.opcode;
        } 
        
        else if (info.type == "U") {
            if (tokens.size() != 4) {
                throw invalid_argument("U-type expects 2 operands");
            }
            uint8_t rd = parseRegister(tokens[2]);
            int32_t imm = parseImmediate(tokens[3],"U");
            if (imm < -524288 || imm > 524287) {
                throw invalid_argument("U-type immediate out of range");
            }
            uint32_t imm_upper = (static_cast<uint32_t>(imm) & 0xFFFFF) << 12;
            machine_code = imm_upper | (rd << 7) | info.opcode;
        } 
        
        else if (info.type == "UJ") {
            if (tokens.size() != 4) {
                throw invalid_argument("UJ-type expects 2 operands");
            }
            uint8_t rd = parseRegister(tokens[2]);
            int32_t imm = parseImmediate(tokens[3],"UJ");
            if (imm % 2 != 0) {
                throw invalid_argument("UJ offset must be even");
            }
            int32_t shifted_imm = imm / 2;
            if (shifted_imm < -524288 || shifted_imm > 524287) {
                throw invalid_argument("UJ offset out of range");
            }
            uint32_t imm_enc = static_cast<uint32_t>(shifted_imm);
            uint32_t imm20 = (imm_enc >> 19) & 0x1;
            uint32_t imm10_1 = imm_enc & 0x3FF;
            uint32_t imm11 = (imm_enc >> 10) & 0x1;
            uint32_t imm19_12 = (imm_enc >> 11) & 0xFF;
            uint32_t imm_field = (imm20 << 19) | (imm19_12 << 11) | (imm11 << 10) | imm10_1;
            machine_code = (imm_field << 12) | (rd << 7) | info.opcode;
        } else {
            throw invalid_argument("Unsupported instruction type: " + info.type);
        }

        cout << "0x" << hex << machine_code << endl;
        
        // Checking whether the file is open.
        if (output_file.is_open()) {
            output_file <<"0x"<<hex <<machine_code<<" ,"<<line<<endl; // Inserting text.
        }
    
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    }
    output_file.close(); // Close the file object.
    input_file.close();
    }

    //main.cpp ends here
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    processDataSegment("assembly.s", "output.txt");

    // Open output.txt for reading
    std::ifstream inputFileCopy("output.txt");
    if (!inputFileCopy) {
        std::cerr << "Error: Cannot open output.txt (file may not exist)" << std::endl;
        return 1;
    }

    // Open output.mc in append mode
    std::ofstream outputFileCopy("output.mc", std::ios::app);
    if (!outputFileCopy) {
        std::cerr << "Error: Cannot open output.mc (check permissions)" << std::endl;
        return 1;
    }

    std::string line;
    bool fileEmpty = true;

    // Read from output.txt and append to output.mc
    while (std::getline(inputFileCopy, line)) {
        outputFileCopy << line << std::endl;
        fileEmpty = false; // If we read at least one line, the file is not empty
    }

    if (fileEmpty) {
        std::cerr << "Warning: output.txt is empty, nothing was appended." << std::endl;
    } else {
        std::cout << "✅ Data from output.txt has been appended to output.mc successfully!" << std::endl;
    }

    // Close files
    inputFileCopy.close();
    outputFileCopy.close();

    return 0;
}