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
#include <cstdio>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <bitset>
using namespace std;
vector<pair<unsigned int, string>> instructions_sample;
map<string, unsigned int> labels_sample;
unsigned int pcsample = 0x0; // Starting program counter
string OVERALLINPUTFILE = "input.asm";
string OVERALLOUTPUTFILE = "input.mc";
string processedfile = "refined_code.asm";
// Function to check if a line is an instruction (not .data, labels, or empty lines)
int lineType(const string &line)
{
    // 1 for instruction, 2 for text labels, 0 for others
    static bool text = false;
    if (line.find(".text") != string::npos)
    {
        text = true;
        return 0;
    }
    if (line.empty() || line[0] == '.')
        return 0; // Ignore empty lines and directives
    if (line.find(':') != string::npos && line[line.size() - 1] == ':')
        return 0; // Ignore data labels
    if (line.find(':') != string::npos && line[line.size() - 1] != ':')
    {
        // divide in the into two parts - word before: and words afer : and return 2
        size_t pos = line.find(':');
        string label = line.substr(0, pos);
        label.erase(label.find_last_not_of(" ") + 1); // Trim trailing spaces
        // Extract second part (ignoring leading spaces)
        string instruct = line.substr(pos + 1);
        // cout << pcsample << endl;
        // cout << "Label: " << label << " " << "Instruction: " << instruct << endl;
        // if instruction string is empty
        // labels.push_back({pcsample, label});
        labels_sample[label] = pcsample;
        if (!instruct.empty())
        {
            instructions_sample.push_back({pcsample, instruct});
            stringstream ss(instruct);
            string opcode;
            ss >> opcode;
            pcsample += 4;
        }
        string after = line.substr(pos + 1);
        return 2;
    }
    return 1;
}

const uint32_t DATA_SEGMENT_START = 0x10000000;

void processDataSegment(const string &inputFileName, const string &outputFileName)
{

    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName);
    outputFile << '\n'
               << "Memory Address   " << "Value" << endl;

    if (!inputFile.is_open() || !outputFile.is_open())
    {
        cerr << "Error opening file!" << endl;
        return;
    }

    string line;
    uint32_t currentAddress = DATA_SEGMENT_START;
    bool inDataSection = false;

    while (getline(inputFile, line))
    {
        istringstream iss(line);
        string word;

        if (line.find(".data") != string::npos)
        {
            inDataSection = true;
            continue;
        }

        if (inDataSection)
        {
            size_t pos;
            if ((pos = line.find(".word")) != string::npos)
            {
                string str = line.substr(pos + 5);
                replace(str.begin(), str.end(), ',', ' ');

                istringstream values(str);
                int num;
                while (values >> num)
                {
                    outputFile << "0x" << hex << uppercase << currentAddress << "     0x"
                               << setw(8) << setfill('0') << num << endl;
                    currentAddress += 4;
                }
            }
            else if ((pos = line.find(".dword")) != string::npos)
            {
                string str = line.substr(pos + 6);
                replace(str.begin(), str.end(), ',', ' ');
                istringstream values(str);
                long long num;
                while (values >> num)
                {
                    outputFile << "0x" << hex << uppercase << currentAddress << "     0x"
                               << setw(16) << setfill('0') << num << endl;
                    currentAddress += 8;
                }
            }
            else if ((pos = line.find(".half")) != string::npos)
            {
                string str = line.substr(pos + 5);
                replace(str.begin(), str.end(), ',', ' ');
                istringstream values(str);
                int num;
                while (values >> num)
                {
                    outputFile << "0x" << hex << uppercase << currentAddress << "     0x"
                               << setw(4) << setfill('0') << num << endl;
                    currentAddress += 2;
                }
            }
            else if ((pos = line.find(".byte")) != string::npos)
            {
                string str = line.substr(pos + 5);
                replace(str.begin(), str.end(), ',', ' ');
                istringstream values(str);
                int num;
                while (values >> num)
                {
                    outputFile << "0x" << hex << uppercase << currentAddress << "     0x"
                               << setw(2) << setfill('0') << num << endl;
                    currentAddress += 1;
                }
            }
            else if ((pos = line.find(".asciz")) != string::npos)
            {
                size_t start = line.find('"', pos);
                size_t end = line.rfind('"');
                if (start != string::npos && end != string::npos && start < end)
                {
                    string str = line.substr(start + 1, end - start - 1);
                    for (char c : str)
                    {
                        outputFile << "0x" << hex << uppercase << currentAddress << "     0x"
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

struct InstructionInfo
{
    string type;
    uint8_t opcode;
    uint8_t funct3;
    uint8_t funct7;
};
string pc;
map<string, int> labels;
map<string, InstructionInfo> instructionMap = {
    //"name" {"type","opcode","funct3",funct7"}
    {"add", {"R", 0x33, 0x0, 0x00}},
    {"sub", {"R", 0x33, 0x0, 0x20}},
    {"sll", {"R", 0x33, 0x1, 0x00}},
    {"slt", {"R", 0x33, 0x2, 0x00}},
    {"sltu", {"R", 0x33, 0x3, 0x00}},
    {"xor", {"R", 0x33, 0x4, 0x00}},
    {"srl", {"R", 0x33, 0x5, 0x00}},
    {"sra", {"R", 0x33, 0x5, 0x20}},
    {"or", {"R", 0x33, 0x6, 0x00}},
    {"and", {"R", 0x33, 0x7, 0x00}},
    {"mul", {"R", 0x33, 0x0, 0x01}},
    {"div", {"R", 0x33, 0x4, 0x01}},
    {"rem", {"R", 0x33, 0x6, 0x01}},

    {"addi", {"I", 0x13, 0x0, 0x00}},
    {"andi", {"I", 0x13, 0x7, 0x00}},
    {"ori", {"I", 0x13, 0x6, 0x00}},
    {"jalr", {"I", 0x67, 0x0, 0x00}},
    {"xori", {"I", 0x13, 0x4, 0x00}},
    {"lw", {"I", 0x03, 0x2, 0x00}},
    {"slli", {"I", 0x13, 0x1, 0x00}},
    {"srai", {"I", 0x13, 0x5, 0x20}},
    {"srli", {"I", 0x13, 0x5, 0x00}},

    {"sw", {"S", 0x23, 0x2, 0x00}},
    {"sb", {"S", 0x23, 0x0, 0x00}},
    {"sd", {"S", 0x23, 0x3, 0x00}},
    {"sh", {"S", 0x23, 0x1, 0x00}},

    {"beq", {"SB", 0x63, 0x0, 0x00}},
    {"bne", {"SB", 0x63, 0x1, 0x00}},
    {"bge", {"SB", 0x63, 0x5, 0x00}},
    {"blt", {"SB", 0x63, 0x4, 0x00}},
    {"lui", {"U", 0x37, 0x0, 0x00}},
    {"auipc", {"U", 0x17, 0x0, 0x00}},
    {"jal", {"UJ", 0x6F, 0x0, 0x00}},
    {"lb", {"I", 0x03, 0x0, 0x00}},
    {"lh", {"I", 0x03, 0x1, 0x00}},
    {"ld", {"I", 0x03, 0x3, 0x00}},

};

unordered_map<string, string> registerMap = {
    {"t0", "x5"}, {"t1", "x6"}, {"t2", "x7"}, {"s0", "x8"}, {"s1", "x9"}, {"a0", "x10"}, {"a1", "x11"}, {"a2", "x12"}, {"a3", "x13"}, {"a4", "x14"}, {"a5", "x15"}, {"a6", "x16"}, {"a7", "x17"}, {"s2", "x18"}, {"s3", "x19"}, {"s4", "x20"}, {"s5", "x21"}, {"s6", "x22"}, {"s7", "x23"}, {"s8", "x24"}, {"s9", "x25"}, {"s10", "x26"}, {"s11", "x27"}, {"t3", "x28"}, {"t4", "x29"}, {"t5", "x30"}, {"t6", "x31"}};

vector<string> tokenize(const string &line)
{
    vector<string> tokens;
    string token;
    istringstream iss(line); // Convert the string `line` into a stream object
    while (iss >> token)
    { // Extract tokens from the stream (separated by spaces)
        if (token[0] == '#')
            break;
        size_t pos;
        while ((pos = token.find(',')) != string::npos)
        {                        // Find ',' in token
            token.erase(pos, 1); // Remove ',' from the token
        }
        tokens.push_back(token);
    }

    return tokens;
}

uint8_t parseRegister(const string &reg)
{
    if (reg.empty() || !(reg[0] == 'x' || reg[0] == 't' || reg[0] == 'a'))
    {
        throw invalid_argument("Invalid register: " + reg);
    }

    int num;
    try
    {
        num = stoi(reg.substr(1));
    }
    catch (...)
    {
        throw invalid_argument("Invalid register: " + reg);
    }
    if (reg[0] == 'x')
    {
        if (num < 0 || num > 31)
        {
            throw invalid_argument("Register out of range: " + reg);
        }
    }
    else if (reg[0] == 't')
    {
        if (num < 0 || num > 6)
        {
            throw invalid_argument("Register out of range: " + reg);
        }
        // reg in registermap
        auto str = registerMap[reg];
        num = stoi(str.substr(1));
    }
    else if (reg[0] == 'a')
    {
        if (num < 0 || num > 10)
        {
            throw invalid_argument("Register out of range: " + reg);
        }
        auto str = registerMap[reg];
        num = stoi(str.substr(1));
    }

    return static_cast<uint8_t>(num);
}

int32_t parseImmediate(const string &immStr, string type)
{
    size_t pos = 0;
    int32_t imm;
    try
    {
        if (immStr.size() > 2 && immStr.substr(0, 2) == "0x")
        {
            imm = stoi(immStr, &pos, 16);
        }
        else if (immStr.size() > 2 && immStr.substr(0, 2) == "0b")
        {
            // cout<<immStr.substr(2)<<endl;
            imm = stoi(immStr.substr(2), &pos, 2);
            pos += 2;
        }
        else if (type == "UJ" || type == "SB")
        {
            // cout << "type: " << type << endl;
            int num = stoi(pc, 0, 16);
            int num2 = labels[immStr];
            if (num2 == 0)
            {
                throw invalid_argument("Label not found: " + immStr);
            }
            imm = num2 - num;

            // cout << dec << imm << endl;
            // cout << hex << imm << endl;
            return imm;
        }
        else
        {
            imm = stoi(immStr, &pos);
        }
    }
    catch (...)
    {
        throw invalid_argument("Invalid immediate: " + immStr);
    }
    if (pos != immStr.size())
    {
        throw invalid_argument("Invalid immediate: " + immStr);
    }
    return imm;
}

// This function extracts and returns the instruction fields in the format:
// <opcode>-<func3>-<func7>-<rd>-<rs1>-<rs2>-<immediate>
// Where the immediate is:
//   - "NULL" for R-type
//   - 12-bit binary for I, S, SB types
//   - 20-bit binary for U, UJ types
string extractInstructionFields(const string &instr)
{
    // Tokenize the input instruction string.
    vector<string> tokens = tokenize(instr);
    if (tokens.empty())
    {
        throw invalid_argument("Empty instruction");
    }

    // The first token is the operation.
    string op = tokens[0];

    if (instructionMap.find(op) == instructionMap.end())
    {
        throw invalid_argument("Unknown instruction: " + op);
    }
    const InstructionInfo &info = instructionMap[op];

    // Get the basic encoding fields from the map.
    uint32_t opcode = info.opcode; // 7 bits
    uint32_t func3 = info.funct3;  // 3 bits
    uint32_t func7 = info.funct7;  // 7 bits

    // Initialize registers and immediate.
    uint32_t rd = 0, rs1 = 0, rs2 = 0;
    int32_t imm = 0; // signed immediate
    // Decode based on the instruction type.
    if (info.type == "R")
    {
        // R-type format: op rd rs1 rs2
        if (tokens.size() != 4)
        {
            throw invalid_argument("R-type expects 3 operands");
        }
        rd = parseRegister(tokens[1]);
        rs1 = parseRegister(tokens[2]);
        rs2 = parseRegister(tokens[3]);
        // R-type has no immediate -> handled below as imm_bin = "NULL"
    }
    else if (info.type == "I")
    {
        // I-type format: "op rd rs1 imm" or "op rd imm(rs1)" for loads
        if (tokens.size() != 4 && tokens.size() != 3)
        {
            throw invalid_argument("I-type expects 2 or 3 operands");
        }

        // If we have 3 tokens, convert "imm(rs1)" into separate imm + rs1
        if (tokens.size() == 3)
        {
            vector<string> newTokens(4);
            newTokens[0] = tokens[0];
            newTokens[1] = tokens[1];
            newTokens[2] = tokens[2].substr(0, tokens[2].find('('));
            newTokens[3] = tokens[2].substr(tokens[2].find('(') + 1, tokens[3].rfind(')') - tokens[3].find('(') - 1);
            tokens = newTokens;
        }

        if (op == "lw" || op == "ld" || op == "lh" || op == "lb")
        {
            rd = parseRegister(tokens[1]);
            rs1 = parseRegister(tokens[3]);
            imm = parseImmediate(tokens[2], "I");
        }
        else
        {
            rd = parseRegister(tokens[1]);
            rs1 = parseRegister(tokens[2]);
            imm = parseImmediate(tokens[3], "I");
        }
    }
    else if (info.type == "S")
    {
        // S-type (store): could be "op rs2 offset(rs1)" or "op rs2 offset rs1"
        if (tokens.size() != 3 && tokens.size() != 4)
        {
            throw invalid_argument("S-type expects 2 or 3 operands");
        }
        string memToken;
        if (tokens.size() == 3)
        {
            rs2 = parseRegister(tokens[1]);
            memToken = tokens[2];
        }
        else
        {
            // Merge offset and base register into one token: offset(rs1)
            rs2 = parseRegister(tokens[1]);
            memToken = tokens[2] + "(" + tokens[3] + ")";
        }
        size_t lparen = memToken.find('(');
        size_t rparen = memToken.find(')');
        if (lparen == string::npos || rparen == string::npos || lparen >= rparen)
        {
            throw invalid_argument("Invalid memory operand format in S-type");
        }
        string offsetStr = memToken.substr(0, lparen);
        string baseRegStr = memToken.substr(lparen + 1, rparen - lparen - 1);
        imm = parseImmediate(offsetStr, "S");
        rs1 = parseRegister(baseRegStr);
    }
    else if (info.type == "SB")
    {
        // SB-type (branch): op rs1 rs2 offset
        if (tokens.size() != 4)
        {
            throw invalid_argument("SB-type expects 3 operands");
        }
        rs1 = parseRegister(tokens[1]);
        rs2 = parseRegister(tokens[2]);
        imm = parseImmediate(tokens[3], "SB");
    }
    else if (info.type == "U")
    {
        // U-type: op rd imm (e.g. lui, auipc)
        if (tokens.size() != 3)
        {
            throw invalid_argument("U-type expects 2 operands");
        }
        rd = parseRegister(tokens[1]);
        imm = parseImmediate(tokens[2], "U");
    }
    else if (info.type == "UJ")
    {
        // UJ-type: op rd offset (e.g. jal)
        if (tokens.size() != 3)
        {
            throw invalid_argument("UJ-type expects 2 operands");
        }
        rd = parseRegister(tokens[1]);
        imm = parseImmediate(tokens[2], "UJ");
    }
    else
    {
        throw invalid_argument("Unsupported instruction type: " + info.type);
    }

    // -------------------------
    // Convert each field to binary or "NULL" if unused.
    // -------------------------

    // 1) opcode always 7 bits
    string opcode_bin = bitset<7>(opcode).to_string();

    // 2) func3 can be 3 bits or "NULL"
    // 3) func7 can be 7 bits or "NULL"
    // 4) rd, rs1, rs2 can be 5 bits or "NULL"
    // 5) immediate is handled separately (12 or 20 bits, or "NULL")
    // First, convert everything to bitsets by default.
    string func3_bin = bitset<3>(func3).to_string();
    string func7_bin = bitset<7>(func7).to_string();
    string rd_bin = bitset<5>(rd).to_string();
    string rs1_bin = bitset<5>(rs1).to_string();
    string rs2_bin = bitset<5>(rs2).to_string();

    // Then override with "NULL" if the instruction type doesn't use them:
    if (info.type == "I")
    {
        func7_bin = "NULL"; // I-type does not use func7
        rs2_bin = "NULL";   // I-type does not use rs2
    }
    else if (info.type == "S")
    {
        rd_bin = "NULL"; // S-type does not use rd
    }
    else if (info.type == "SB")
    {
        rd_bin = "NULL"; // SB-type (branch) does not use rd
    }
    else if (info.type == "U" || info.type == "UJ")
    {
        // U and UJ do not use rs1, rs2, func3, func7
        rs1_bin = "NULL";
        rs2_bin = "NULL";
        func3_bin = "NULL";
        func7_bin = "NULL";
    }
    // R-type keeps them all as bitsets (already correct from above).

    // -------------------------
    // Determine immediate size or "NULL"
    // -------------------------
    string imm_bin;
    if (info.type == "R")
    {
        imm_bin = "NULL";
    }
    else if (info.type == "I" || info.type == "S" || info.type == "SB")
    {
        // 12-bit immediate
        uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFF; // Lower 12 bits
        imm_bin = bitset<12>(imm12).to_string();
    }
    else if (info.type == "U" || info.type == "UJ")
    {
        // 20-bit immediate
        uint32_t imm20 = static_cast<uint32_t>(imm) & 0xFFFFF; // Lower 20 bits
        imm_bin = bitset<20>(imm20).to_string();
    }

    // -------------------------
    // Assemble final string
    // -------------------------
    string result = opcode_bin + "-" +
                    func3_bin + "-" +
                    func7_bin + "-" +
                    rd_bin + "-" +
                    rs1_bin + "-" +
                    rs2_bin + "-" +
                    imm_bin;

    return result;
}

int main()
{
    ifstream inputFileSample(OVERALLINPUTFILE); // Input assembly file
    ofstream outputFileSample(processedfile);   // Output file with PC

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
    while (((line_sample.find('#') != string::npos && line_sample.find(".data") == string::npos) || line_sample.size() == 0) && getline(inputFileSample, line_sample))
        getline(inputFileSample, line_sample);
    // check if we are at EOF
    if (inputFileSample.eof())
    {
        // go to start of file
        inputFileSample.clear();
        inputFileSample.seekg(0, ios::beg);
        getline(inputFileSample, line_sample);
    }
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
    }
    do
    {
        // Remove comments from the line_sample
        size_t commentPos = line_sample.find('#');
        if (commentPos != string::npos)
        {
            line_sample = line_sample.substr(0, commentPos);
        }
        replace(line_sample.begin(), line_sample.end(), ',', ' ');
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
                    itislabel = false;
                    try
                    {
                        stoi(label);
                    }
                    catch (...)
                    {
                        itislabel = true;
                    }
                }
                if (itislabel)
                {
                    // cout<<label<<endl;
                    if (labelmap.find(label) == labelmap.end())
                    {
                        cerr << "Error: Label not found" << endl;
                        return 1;
                    }
                    instructions_sample.push_back({pcsample, "auipc " + reg + " 65536"});
                    pcsample += 4;
                    instructions_sample.push_back({pcsample, opname + " " + reg + " " + to_string(labelmap[label] - 268435456 - (pcsample - 4)) + "(" + reg + ")"});
                    pcsample += 4;
                }
                else
                {
                    // cout<<"here:"<<line_sample<<endl;
                    instructions_sample.push_back({pcsample, line_sample});
                    pcsample += 4;
                }
            }
            else if (opname == "la")
            {
                string reg;
                ss >> reg;
                string label;
                ss >> label;

                if (labelmap.find(label) == labelmap.end())
                {
                    cerr << "Error: Label not found" << endl;
                    return 1;
                }
                instructions_sample.push_back({pcsample, "auipc " + reg + " 65536"});
                pcsample += 4;
                instructions_sample.push_back({pcsample, "addi " + reg + " " + reg + " " + to_string(labelmap[label] - 268435456 - (pcsample - 4))});
                pcsample += 4;
            }
            else if (opname == "j")
            {
                string label;
                ss >> label;
                instructions_sample.push_back({pcsample, "jal x0 " + label});
                pcsample += 4;
            }
            else if (opname == "li")
            {
                string reg;
                ss >> reg;
                string imm;
                ss >> imm;
                // check if imm is within 12 bits (signed)
                int imm_val = stoi(imm);
                if (imm_val > 2047 || imm_val < -2048)
                {
                    instructions_sample.push_back({pcsample, "addi " + reg + " x0 " + imm});
                    pcsample += 4;
                }
                else
                {
                    instructions_sample.push_back({pcsample, "lui " + reg + " " + to_string(imm_val >> 12)});
                    pcsample += 4;
                    instructions_sample.push_back({pcsample, "addi " + reg + " " + reg + " " + to_string(imm_val & 0xFFF)});
                    pcsample += 4;
                }
            }
            else if (opname == "beqz")
            {
                string reg;
                ss >> reg;
                string label;
                ss >> label;
                instructions_sample.push_back({pcsample, "beq " + reg + " x0 " + label});
                pcsample += 4;
            }
            else if (opname == "bnez")
            {
                string reg;
                ss >> reg;
                string label;
                ss >> label;
                instructions_sample.push_back({pcsample, "bne " + reg + " x0 " + label});
                pcsample += 4;
            }
            else
            {
                instructions_sample.push_back({pcsample, line_sample});
                pcsample += 4;
            }
        }
        else if (type == 2)
        {
            labels[line_sample.substr(0, line_sample.find(':'))] = pcsample;
            instructions_sample.push_back({pcsample, line_sample.substr(line_sample.find(':') + 1, line_sample.size() - 1)});
            pcsample += 4;
        }
        else
        {
            if (line_sample.find(':') != string::npos)
            {
                labels[line_sample.substr(0, line_sample.find(':'))] = pcsample;
            }
        }
    } while (getline(inputFileSample, line_sample));

    // Write to output file with program counter
    for (const auto &pair : instructions_sample)
    {
        outputFileSample << "0x" << hex << pair.first << ": " << pair.second << endl;
        // outputFileSample << "0x" << hex << setw(8) << setfill('0') << pair.first << ": " << pair.second << endl;
    }
    outputFileSample << endl;

    // print label with pcsample
    for (const auto &pair : labels_sample)
    {
        outputFileSample << "0x" << hex << setw(8) << setfill('0') << pair.first << ": " << pair.second << endl;
    }

    cout << "Processed assembly instructions saved to refined_code.asm" << endl;
    inputFileSample.close();
    outputFileSample.close();

    // main.cpp starts here

    fstream input_file, output_file;
    input_file.open(processedfile, ios::in);

    if (input_file.is_open())
    {
        string line;
        output_file.open(OVERALLOUTPUTFILE, ios::out);
        // Read data from the file object and put it into a string.
        output_file << "Address       Machine Code    Assembly Code \t\t\t\t\t   Opcode-Func3-Func7-rd-rs1-imm" << endl;
        while (getline(input_file, line))
        {
            vector<string> tokens = tokenize(line);
            if (tokens.empty())
            {
                break;
            }
            pc = tokens[0];
            string op = tokens[1];
            if (instructionMap.find(op) == instructionMap.end())
            {
                cerr << "Error: Unknown instruction '" << op << "'" << endl;
                return 1;
            }

            const auto &info = instructionMap[op];

            try
            {
                uint32_t machine_code = 0;
                if (info.type == "R")
                {
                    if (tokens.size() != 5)
                    {
                        throw invalid_argument("R-type expects 3 operands");
                    }
                    uint8_t rd = parseRegister(tokens[2]);
                    uint8_t rs1 = parseRegister(tokens[3]);
                    uint8_t rs2 = parseRegister(tokens[4]);
                    machine_code = (info.funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (info.funct3 << 12) | (rd << 7) | info.opcode;
                }
                else if (info.type == "I")
                {
                    if (tokens.size() != 5 && tokens.size() != 4)
                    {

                        throw invalid_argument("I-type expects 3 operands");
                    }
                    if (tokens.size() == 4)
                    {
                        vector<string> newTokens(5);
                        newTokens[0] = tokens[0];
                        newTokens[1] = tokens[1];
                        newTokens[2] = tokens[2];
                        newTokens[3] = tokens[3].substr(0, tokens[3].find('('));
                        newTokens[4] = tokens[3].substr(tokens[3].find('(') + 1, tokens[3].rfind(')') - tokens[3].find('(') - 1);
                        tokens = newTokens;
                    }

                    uint32_t rd, rs1;
                    int32_t imm;
                    if (op == "lw" || op == "ld" || op == "lh" || op == "lb")
                    {
                        rd = parseRegister(tokens[2]);
                        rs1 = parseRegister(tokens[4]);
                        imm = parseImmediate(tokens[3], "I");
                    }
                    else
                    {
                        rd = parseRegister(tokens[2]);
                        rs1 = parseRegister(tokens[3]);
                        imm = parseImmediate(tokens[4], "I");
                    }
                    if (imm < -2048 || imm > 2047)
                    {
                        throw invalid_argument("I-type immediate out of range");
                    }
                    uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFF;
                    machine_code = (imm12 << 20) | (rs1 << 15) | (info.funct3 << 12) | (rd << 7) | info.opcode;
                }
                else if (info.type == "S")
                {
                    if (tokens.size() != 4 && tokens.size() != 5)
                    {
                        throw invalid_argument("S-type expects 3 operands");
                    }
                    if (tokens.size() == 5)
                    {
                        vector<string> newtokens(4);
                        newtokens[0] = tokens[0];
                        newtokens[1] = tokens[1];
                        newtokens[2] = tokens[2];
                        newtokens[3] = tokens[3] + "(" + tokens[4] + ")";
                        tokens = newtokens;
                    }

                    uint8_t rs2 = parseRegister(tokens[2]);
                    string mem = tokens[3];
                    size_t lparen = mem.find('(');
                    size_t rparen = mem.find(')');
                    if (lparen == string::npos || rparen == string::npos || lparen >= rparen)
                    {
                        throw invalid_argument("Invalid memory operand format");
                    }
                    int32_t imm = parseImmediate(mem.substr(0, lparen), "S");
                    uint8_t rs1 = parseRegister(mem.substr(lparen + 1, rparen - lparen - 1));
                    if (imm < -2048 || imm > 2047)
                    {
                        throw invalid_argument("S-type immediate out of range");
                    }
                    uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFF;
                    uint32_t imm_high = (imm12 >> 5) & 0x7F;
                    uint32_t imm_low = imm12 & 0x1F;
                    machine_code = (imm_high << 25) | (rs2 << 20) | (rs1 << 15) | (info.funct3 << 12) | (imm_low << 7) | info.opcode;
                }

                else if (info.type == "SB")
                {
                    if (tokens.size() != 5)
                    {
                        throw invalid_argument("SB-type expects 3 operands");
                    }
                    uint8_t rs1 = parseRegister(tokens[2]);
                    uint8_t rs2 = parseRegister(tokens[3]);
                    int32_t imm = parseImmediate(tokens[4], "SB");

                    if (imm % 2 != 0)
                    {
                        throw invalid_argument("SB offset must be even");
                    }
                    int32_t shifted_imm = imm;
                    if (shifted_imm < -4096 || shifted_imm > 4095)
                    {
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

                else if (info.type == "U")
                {
                    if (tokens.size() != 4)
                    {
                        throw invalid_argument("U-type expects 2 operands");
                    }
                    uint8_t rd = parseRegister(tokens[2]);
                    int32_t imm = parseImmediate(tokens[3], "U");
                    if (imm < -1048577 || imm > 1048576)
                    {
                        // cout << imm << endl;
                        throw invalid_argument("U-type immediate out of range");
                    }
                    uint32_t imm_upper = (static_cast<uint32_t>(imm) & 0xFFFFF) << 12;
                    machine_code = imm_upper | (rd << 7) | info.opcode;
                }

                else if (info.type == "UJ")
                {
                    if (tokens.size() != 4)
                    {
                        throw invalid_argument("UJ-type expects 2 operands");
                    }
                    uint8_t rd = parseRegister(tokens[2]);
                    int32_t imm = parseImmediate(tokens[3], "UJ");
                    if (imm % 2 != 0)
                    {
                        throw invalid_argument("UJ offset must be even");
                    }
                    if (imm < -524288 || imm > 524287)
                    { // Ensure 20-bit signed range
                        throw invalid_argument("UJ offset out of range");
                    }

                    uint32_t imm_enc = static_cast<uint32_t>(imm);

                    uint32_t imm20 = (imm_enc >> 20) & 0x1;     // Extract imm[20] (sign bit)
                    uint32_t imm10_1 = (imm_enc >> 1) & 0x3FF;  // Extract imm[10:1]
                    uint32_t imm11 = (imm_enc >> 11) & 0x1;     // Extract imm[11]
                    uint32_t imm19_12 = (imm_enc >> 12) & 0xFF; // Extract imm[19:12]

                    // Correctly arrange the immediate fields
                    uint32_t imm_field = (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) | (imm10_1 << 21);

                    // Generate final machine code
                    machine_code = imm_field | (rd << 7) | info.opcode;
                }
                else
                {
                    throw invalid_argument("Unsupported instruction type: " + info.type);
                }

                // cout << "0x" << hex << machine_code << endl;
                //  Checking whether the file is open.
                if (output_file.is_open())
                {
                    // output_file << "0x" << hex << machine_code << " ," << line << endl; // Inserting text.
                    // output_file << line.substr(0, line.find(' ')) << "    0x" << hex << std::setw(8) << std::setfill('0') << machine_code << "      " << line.substr(11, line.size() - 1) << "    # " << extractInstructionFields(line.substr(line.find(' ') + 1)) << endl;
                    output_file << std::left << std::setw(13) << line.substr(0, line.find(' '))
                                << std::setfill(' ')
                                << " 0x" << std::right << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << machine_code
                                << std::setfill(' ') << std::left // switch back to left-justification for the following fields
                                << "      " << std::setw(35) << line.substr(line.find(' ') + 1)
                                << "  # " << extractInstructionFields(line.substr(line.find(' ') + 1))
                                << std::endl;
                }
            }
            catch (const exception &e)
            {
                cerr << "Error: " << e.what() << endl;
                return 1;
            }
        }
        output_file << "0x8   #end of text segment" << endl;
        output_file.close(); // Close the file object.
        input_file.close();
    }

    // main.cpp ends here

    // std::this_thread::sleep_for(std::chrono::seconds(1));
    processDataSegment(OVERALLINPUTFILE, "output.txt");

    // Open output.txt for reading
    std::ifstream inputFileCopy("output.txt");
    if (!inputFileCopy)
    {
        std::cerr << "Error: Cannot open output.txt (file may not exist)" << std::endl;
        return 1;
    }

    // Open output.mc in append mode
    std::ofstream outputFileCopy(OVERALLOUTPUTFILE, std::ios::app);
    if (!outputFileCopy)
    {
        std::cerr << "Error: Cannot open input.mc (check permissions)" << std::endl;
        return 1;
    }

    std::string line;
    bool fileEmpty = true;

    // Read from output.txt and append to output.mc
    while (std::getline(inputFileCopy, line))
    {
        outputFileCopy << line << std::endl;
        fileEmpty = false; // If we read at least one line, the file is not empty
    }

    if (fileEmpty)
    {
        std::cerr << "Warning: output.txt is empty, nothing was appended." << std::endl;
    }
    else
    {
        std::cout << "Data from output.txt has been appended to input.mc successfully!" << std::endl;
    }

    // Close files
    inputFileCopy.close();
    outputFileCopy.close();

    return 0;
}