#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cstdint>
#include <bitset>
#include <unordered_map>
using namespace std;

uint32_t global_pc = 0x0;
int RM = 0x0;
int RZ = 0x0;
int RY = 0x0;
int EX_MEM,MEM_WB;
int alu_input1 = 0,alu_input2 = 0,alu_signal;

struct Instruction
{
    uint32_t mc;

    string needs_rs1_in;
    string needs_rs2_in;
    string op;

    int rd;
    int rs1;
    int rs2;
    int32_t imm;
    bool rs1_needs_EX_to_EX = false;
    bool rs2_needs_EX_to_EX = false;
    bool rs1_needs_MEM_to_EX = false;
    bool rs2_needs_MEM_to_EX = false;
    bool rs1_needs_MEM_to_MEM = false;
    bool rs2_needs_MEM_to_MEM = false;
    bool stall = false;
    
    
    bool dependent_rs1 = false,dependent_rs2 = false;
};

struct PipelineStage
{
    Instruction *instr = nullptr;};

bool needsForwarding( Instruction &curr,  Instruction &prev, string in)
{
    if (prev.op == "NOP" || prev.rd == -1)
        return false;
        if((curr.rs1 == prev.rd && curr.needs_rs1_in == in)){
            curr.dependent_rs1 = true;
            return true;}

    if((curr.rs2 == prev.rd && curr.needs_rs2_in == in)){
        curr.dependent_rs2 = true;
        return true;
    }
    return false;
}

bool loadUseHazard(const Instruction &curr, const Instruction &prev)
{

    return (prev.op == "LW" || prev.op == "LH" || prev.op == "LB") &&
           ((curr.rs1 == prev.rd && curr.needs_rs1_in == "EX") || (curr.rs2 == prev.rd && curr.needs_rs2_in == "EX"));
}
unordered_map<string, int> operationMap = {
    {"ADD", 1}, {"ADDI", 2}, {"LB", 3}, {"LD", 4}, {"LH", 5}, {"LW", 6}, {"JALR", 7}, {"JAL", 8}, {"SB", 9}, {"SH", 10}, {"SD", 11}, {"SW", 12}, {"BEQ", 13}, {"BNE", 14}, {"BGE", 15}, {"BLT", 16}, {"AND", 17}, {"ANDI", 18}, {"OR", 19}, {"ORI", 20}, {"MUL", 21}, {"DIV", 22}, {"REM", 23}, {"XOR", 24}, {"SUB", 25}, {"SLL", 26}, {"SLT", 27}, {"SRL", 28}, {"SRA", 29}, {"LUI", 30}, {"AUIPC", 31}};

uint32_t ALU(int val1, int val2, int OP)
{
    if (OP == 1 || OP == 2 || OP == 3 || OP == 4 || OP == 5 || OP == 6 || OP == 7 || OP == 8 || OP == 9 || OP == 10 || OP == 11 || OP == 12)
    {
        return val1 + val2;
    }
    else if (OP == 13)
    {

        return val1 == val2;
    }
    else if (OP == 14)
        return val1 != val2;
    else if (OP == 15)
        return val1 >= val2;
    else if (OP == 16)
        return val1 < val2;
    else if (OP == 17 || OP == 18)
        return RZ = val1 & val2;
    else if (OP == 19 || OP == 20)
        return RZ = val1 | val2;
    else if (OP == 21)
        return RZ = val1 * val2;
    else if (OP == 22)
        if (val2 == 0)
        {
            throw runtime_error("Division by zero error");
            return 0;
        }
        else
            return RZ = val1 / val2;
    else if (OP == 23)
        return RZ = val1 % val2;
    else if (OP == 24)
        return RZ = val1 ^ val2;
    else if (OP == 25)
        return RZ = val1 - val2;
    else if (OP == 26)
        return RZ = val1 << val2;
    else if (OP == 27)
        return RZ = val1 < val2;
    else if (OP == 28)
        return RZ = val1 >> val2;
    else if (OP == 29)
    {
        RZ = (int32_t)(val1 >> val2);
        return RZ;
    }
    else if (OP == 30)
    {
        return val1 << 12;
    }
    else if (OP == 31)
    {
        return (val1 << 12) + val2;
    }
    else
    {
        cout << "Invalid Operation in ALU" << endl;
        return 0;
    }
}

unordered_map<string, string> rTypeInstructions = {
    {"01100110000000000", "ADD"},
    {"01100110000100000", "SUB"},
    {"01100110010000000", "SLL"},
    {"01100110100000000", "SLT"},
    {"01100111010100000", "SRA"},
    {"01100111000000001", "DIV"},
    {"01100111000000000", "XOR"},
    {"01100111010000000", "SRL"},
    {"01100111100000000", "OR"},
    {"01100111110000000", "AND"},
    {"01100110000000001", "MUL"},
    {"01100111100000001", "REM"}};

unordered_map<string, string> iTypeInstructions = {
    {"0010011000", "ADDI"},
    {"0010011010", "SLTI"},
    {"0010011100", "XORI"},
    {"0010011110", "ORI"},
    {"0010011111", "ANDI"},
    {"0000011010", "LW"},
    {"0000011000", "LB"},
    {"0000011001", "LH"},
    {"0000011011", "LD"},
    {"1100111000", "JALR"}};

unordered_map<string, string> sTypeInstructions = {
    {"0100011000", "SB"},
    {"0100011001", "SH"},
    {"0100011010", "SW"},
    {"0100011011", "SD"}};

unordered_map<string, string> sbTypeInstructions = {
    {"1100011000", "BEQ"},
    {"1100011001", "BNE"},
    {"1100011100", "BLT"},
    {"1100011101", "BGE"}};

unordered_map<string, string> uTypeInstructions = {
    {"0010111", "AUIPC"},
    {"0110111", "LUI"}};

unordered_map<string, string> ujTypeInstructions = {
    {"1101111", "JAL"}};

vector<int> RegFile = {0, 0, 2147483612, 268435456, 0, 0, 0, 0, 0, 0, 1, 2147483612, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const int regNums[32] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

Instruction decodeRType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;
    Instruction curr;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string() + bitset<7>(funct7).to_string();
    if (rTypeInstructions.find(key) != rTypeInstructions.end())
    {
        // {instruction,"EX","EX",rTypeInstructions[key], regNums[rd], regNums[rs1], regNums[rs2]};
        curr.mc = instruction;
        curr.needs_rs1_in = "EX";
        curr.needs_rs2_in = "EX";
        curr.op = rTypeInstructions[key];
        alu_signal = operationMap[curr.op];
        curr.rd = regNums[rd];
        curr.rs1 = regNums[rs1];
        curr.rs2 = regNums[rs2];
        curr.imm = 0;
        alu_input1 = RegFile[curr.rs1];
        alu_input2 = RegFile[curr.rs2];
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            curr.stall = true;
        }
        // Execute from previous instruction
        else
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_EX_to_EX = true:curr.rs2_needs_EX_to_EX = true;
                curr.dependent_rs1?curr.dependent_rs1 = false:curr.dependent_rs2 = false;
                
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_MEM = true:curr.rs2_needs_MEM_to_MEM = true;
                curr.dependent_rs1?curr.dependent_rs1 = false:curr.dependent_rs2 = false;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_EX = true:curr.rs2_needs_MEM_to_EX = true;
                curr.dependent_rs1?curr.dependent_rs1 = false:curr.dependent_rs2 = false;
            }
        }
        if(curr.rs1_needs_EX_to_EX)alu_input1 = EX_MEM;
  if(curr.rs2_needs_EX_to_EX)alu_input2 = EX_MEM;
  if(curr.rs1_needs_MEM_to_EX)alu_input1 = MEM_WB;
  if(curr.rs2_needs_MEM_to_EX)alu_input2 = MEM_WB;

        return curr;
    }
    return {instruction, "Unknown"};
}

Instruction decodeIType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (int32_t)(instruction >> 20);
    if (imm & 0x800)
        imm |= 0xFFFFF000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    Instruction curr;
    if (iTypeInstructions.find(key) != iTypeInstructions.end())
    {
        // curr = {instruction,"EX","",iTypeInstructions[key], regNums[rd], regNums[rs1], imm
        curr.mc = instruction;
        curr.needs_rs1_in = "EX";
        curr.needs_rs2_in = "";
        curr.op = iTypeInstructions[key];
        alu_signal = operationMap[curr.op];
        curr.rd = regNums[rd];
        curr.rs1 = regNums[rs1];
        curr.rs2 = -1;
        curr.imm = imm;
        bool stall = false;
        alu_input1 = RegFile[curr.rs1];
        alu_input2 = curr.imm;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            curr.stall = true;
        }
        // Execute from previous instruction
        else
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_EX_to_EX = true:curr.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_MEM = true:curr.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_EX = true:curr.rs2_needs_MEM_to_EX = true;
            }
        }
        if(curr.rs1_needs_EX_to_EX)alu_input1 = EX_MEM;
        if(curr.rs2_needs_EX_to_EX)alu_input2 = EX_MEM;
        if(curr.rs1_needs_MEM_to_EX)alu_input1 = MEM_WB;
        if(curr.rs2_needs_MEM_to_EX)alu_input2 = MEM_WB;
        return curr;
    }

    return {instruction, "Unknown"};
}

Instruction decodeSType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm4_0 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm11_5 = (instruction >> 25) & 0x7F;
    RM = RegFile[rs2];
    int32_t imm = (imm11_5 << 5) | imm4_0;
    if (imm & (1 << 11))
        imm |= 0xFFFFF000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    Instruction curr;

    if (sTypeInstructions.find(key) != sTypeInstructions.end())
    {
        curr = {instruction, "MEM", "EX", sTypeInstructions[key], -1, regNums[rs1], regNums[rs2], imm};
        alu_signal = operationMap[curr.op];
        alu_input1 = RegFile[curr.rs1];
        alu_input2 = curr.imm;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            curr.stall = true;
        }
        // Execute from previous instruction
        else
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_EX_to_EX = true:curr.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_MEM = true:curr.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_EX = true:curr.rs2_needs_MEM_to_EX = true;
            }
        }
        if(curr.rs1_needs_EX_to_EX)alu_input1 = EX_MEM;
        if(curr.rs2_needs_EX_to_EX)alu_input2 = EX_MEM;
        if(curr.rs1_needs_MEM_to_EX)alu_input1 = MEM_WB;
        if(curr.rs2_needs_MEM_to_EX)alu_input2 = MEM_WB;
        return curr;
    }
    return {instruction, "Unknown"};
}

Instruction decodeSBType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm11 = (instruction >> 7) & 0x1;
    uint32_t imm4_1 = (instruction >> 8) & 0xF;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm10_5 = (instruction >> 25) & 0x3F;
    uint32_t imm12 = (instruction >> 31) & 0x1;
    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    if (imm & (1 << 12))
        imm |= 0xFFFFE000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    Instruction curr;
    if (sbTypeInstructions.find(key) != sbTypeInstructions.end())
    {
        curr = {instruction, "EX", "EX", sbTypeInstructions[key], -1, regNums[rs1], regNums[rs2], imm};
        alu_signal = operationMap[curr.op];
        alu_input1 = RegFile[curr.rs1];
        alu_input2 = RegFile[curr.rs2];
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            curr.stall = true;
        }
        // Execute from previous instruction
        else
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_EX_to_EX = true:curr.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_MEM = true:curr.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_EX = true:curr.rs2_needs_MEM_to_EX = true;
            }
        }
        if(curr.rs1_needs_EX_to_EX)alu_input1 = EX_MEM;
        if(curr.rs2_needs_EX_to_EX)alu_input2 = EX_MEM;
        if(curr.rs1_needs_MEM_to_EX)alu_input1 = MEM_WB;
        if(curr.rs2_needs_MEM_to_EX)alu_input2 = MEM_WB;
        return curr;
    }
    return {instruction, "Unknown"};
}

Instruction decodeUType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    int32_t imm = (instruction >> 12);
    string key = bitset<7>(opcode).to_string();
    Instruction curr;
    if (uTypeInstructions.find(key) != uTypeInstructions.end())
    {
        curr = {instruction, "", "", uTypeInstructions[key], regNums[rd], -1, -1, imm};
        alu_signal = operationMap[curr.op];
        alu_input1 = curr.imm;
        alu_input2 = global_pc;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            curr.stall = true;
        }
        // Execute from previous instruction
        else
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_EX_to_EX = true:curr.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_MEM = true:curr.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_EX = true:curr.rs2_needs_MEM_to_EX = true;
            }
        }
        if(curr.rs1_needs_EX_to_EX)alu_input1 = EX_MEM;
        if(curr.rs2_needs_EX_to_EX)alu_input2 = EX_MEM;
        if(curr.rs1_needs_MEM_to_EX)alu_input1 = MEM_WB;
        if(curr.rs2_needs_MEM_to_EX)alu_input2 = MEM_WB;
        return curr;
    }
    return {instruction, "Unknown"};
}

Instruction decodeUJType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    uint32_t imm19_12 = (instruction >> 12) & 0xFF;
    uint32_t imm11 = (instruction >> 20) & 0x1;
    uint32_t imm10_1 = (instruction >> 21) & 0x3FF;
    uint32_t imm20 = (instruction >> 31) & 0x1;
    int32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    if (imm & (1 << 20))
        imm |= 0xFFE00000;
    string key = bitset<7>(opcode).to_string();
    Instruction curr;
    if (ujTypeInstructions.find(key) != ujTypeInstructions.end())
    {
        curr = {instruction, "", "", ujTypeInstructions[key], regNums[rd], -1, -1, imm};
        alu_signal = operationMap[curr.op];
        alu_input1 = global_pc;
        alu_input2= curr.imm;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            curr.stall = true;
        }
        // Execute from previous instruction
        else
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_EX_to_EX = true:curr.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_MEM = true:curr.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1? curr.rs1_needs_MEM_to_EX = true:curr.rs2_needs_MEM_to_EX = true;
            }
        }
        if(curr.rs1_needs_EX_to_EX)alu_input1 = EX_MEM;
        if(curr.rs2_needs_EX_to_EX)alu_input2 = EX_MEM;
        if(curr.rs1_needs_MEM_to_EX)alu_input1 = MEM_WB;
        if(curr.rs2_needs_MEM_to_EX)alu_input2 = MEM_WB;
        return curr;
    }
    return {instruction, "Unknown"};
}

Instruction decodeInstruction(uint32_t instr, vector<PipelineStage> &pipeline)
{
    uint32_t opcode = instr & 0x7F;
    switch (opcode)
    {
    case 0x33:
        return decodeRType(instr, pipeline);
    case 0x13:
    case 0x03:
    case 0x67:
        return decodeIType(instr, pipeline);
    case 0x23:
        return decodeSType(instr, pipeline);
    case 0x63:
        return decodeSBType(instr, pipeline);
    case 0x37:
    case 0x17:
        return decodeUType(instr, pipeline);
    case 0x6F:
        return decodeUJType(instr, pipeline);
    default:
        return {instr, "Unknown"};
    }
}

int Execute()
{
  
  int val = ALU(alu_input1,alu_input2,alu_signal);
  EX_MEM = val;
  return val;
}

vector<uint32_t> loadProgram(const string &filename)
{
    vector<uint32_t> program;
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        if (line.empty())
            continue;
        uint32_t instr = stoul(line, nullptr, 16);
        program.push_back(instr);
    }
    return program;
}

int main()
{
    vector<uint32_t> program = loadProgram("instructions.txt");

    vector<PipelineStage> pipeline(5); // IF, ID, EX, MEM, WB
    int cycle = 0, pc = 0;
    Instruction nop; // represents a bubble (NOP)

    cout << "Pipeline simulation with forwarding and correct timing:\n\n";

    while (pc < program.size() || any_of(pipeline.begin(), pipeline.end(), [](auto &st) { return st.instr != nullptr; }))
    {
        cout << "Cycle " << ++cycle << ":\n";

        // ----------- STEP 1: Process Stages from WB to EX first -------------
        if (pipeline[4].instr)
            cout << "  WriteBack:  0x" << setfill('0') << setw(8) << hex << pipeline[4].instr->mc << "\n";
        if (pipeline[3].instr)
            cout << "  MemAccess:  0x" << setfill('0') << setw(8) << hex << pipeline[3].instr->mc << "\n";
        if (pipeline[2].instr)
        {
            cout << "  Execute:    0x" << setfill('0') << setw(8) << hex << pipeline[2].instr->mc << "\n";
            int val = Execute(); // This should read only current state (not updated by Decode)
            cout << " VAL " << val << endl;
        }

        // ----------- STEP 2: Hazard Detection & Decode -------------
        bool stall = false;
        if (pipeline[1].instr) // ID stage
        {
            Instruction &curr = *pipeline[1].instr;
            // Decode the instruction and check for hazards
            Instruction decoded = decodeInstruction(curr.mc, pipeline);
            curr = decoded;
            stall = curr.stall;

            cout << "  Decode:     0x" << setfill('0') << setw(8) << hex << curr.mc << "\n";
        }

        // ----------- STEP 3: Fetch Stage -------------
        if (!stall && pc < program.size())
        {
            cout << "  Fetch:      0x" << setfill('0') << setw(8) << hex << program[pc] << "\n";
            pipeline[0].instr = new Instruction();
            pipeline[0].instr->mc = program[pc];
        }

        // ----------- STEP 4: Pipeline Register Update -------------
        if (stall)
        {
            // Insert bubble into EX, keep ID and IF stages
            pipeline[4] = pipeline[3]; // MEM → WB
            pipeline[3] = pipeline[2]; // EX → MEM
            pipeline[2].instr = &nop; // EX becomes bubble

            // Revert PC if an instruction was fetched in this cycle
            if (pipeline[0].instr != nullptr)
            {
                delete pipeline[0].instr;
                pipeline[0].instr = nullptr;
                pc--;
            }

            cout << "STALLING THE PIPELINE!!! " << endl;
        }
        else
        {
            // Normal pipeline advancement
            pipeline[4] = pipeline[3]; // MEM → WB
            pipeline[3] = pipeline[2]; // EX → MEM
            pipeline[2] = pipeline[1]; // ID → EX
            pipeline[1] = pipeline[0]; // IF → ID
            pipeline[0].instr = nullptr; // Clear IF

            pc++; // Increment PC after fetching
        }

        cout << "\n";
    }

    // Cleanup dynamically allocated instructions
    for (auto &stage : pipeline)
    {
        if (stage.instr != nullptr && stage.instr != &nop)
        {
            delete stage.instr;
            stage.instr = nullptr;
        }
    }

    return 0;
}

