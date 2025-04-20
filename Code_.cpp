#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include <unordered_map>
#include <bits/stdc++.h>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cctype>
#include <string>
#include <unordered_set>

using namespace std;

// ─── Phase‑3 “Knobs” ────────────────────────────────────────────────────────
bool knob1 = true;  // Knob1: Enable/disable pipelining
bool knob2 = true;  // Knob2: Enable/disable data forwarding
bool knob3 = false; // Knob3: Print register file each cycle
bool knob4 = false; // Knob4: Print pipeline‑register info each cycle
bool knob5 = true;  // Knob5: Trace a specific instruction
int knob5_num = 2;  //      which instruction index to trace
bool knob6 = false; // Knob6: Print branch‑predictor details

// ─── Phase‑3 Statistics Counters ───────────────────────────────────────────
uint64_t statCycles = 0;            // Stat1
uint64_t statInstCount = 0;         // Stat2
uint64_t statLoadsStores = 0;       // Stat4
uint64_t statALUInst = 0;           // Stat5
uint64_t statControlInst = 0;       // Stat6
uint64_t statStalls = 0;            // Stat7
uint64_t statDataHazards = 0;       // Stat8
uint64_t statControlHazards = 0;    // Stat9
uint64_t statBranchMispredicts = 0; // Stat10
uint64_t statStallsData = 0;        // Stat11
uint64_t statStallsControl = 0;     // Stat12

uint32_t global_pc = 0x0;
int RM = 0x0;
int RZ = 0x0;
int RY = 0x0;
vector<pair<int, uint32_t>> InstructionPCPairs;
vector<int> RegFile = {0, 0, 2147483612, 268435456, 0, 0, 0, 0, 0, 0, 1, 2147483612, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
map<uint32_t, int> MainMemory;

// Branch Target Buffer: maps branch-PC to its predicted target address
static unordered_map<uint32_t, uint32_t> BTB;
bool branchPrediction;
uint32_t predictedPc;
// Branch History Table: maps branch-PC to last outcome (true = taken, false = not taken)
static unordered_map<uint32_t, bool> BHT;

// Helper to test if an opcode corresponds to a branch/jump
inline bool isBranchOpcode(uint32_t opcode)
{
    return opcode == 0x63    // BEQ, BNE, BLT, BGE
           || opcode == 0x6F // JAL
           || opcode == 0x67 // JALR
        ;
}

const int regNums[32] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

void MemAccessforDataSeg(string op, int value, int eff)
{
    if (op == "SB" || op == "SH" || op == "SW")
    {
        unordered_set<int> st;
        int size = 0;
        if (op == "SB")
        {
            size = 1;
        }
        else if (op == "SH")
        {
            size = 2;
        }
        else if (op == "SW")
        {
            size = 4;
        }

        for (int i = 0; i < size; i++)
        {
            MainMemory[eff + i] = static_cast<uint8_t>(value & 0xFF); // Extract lowest 8 bits
            value >>= 8;                                              // Shift right to get next byte
            st.insert(eff + i - (eff + i) % 4);
        }
    }
}

struct IF_ID
{
    uint32_t pc;
    uint32_t instruction;
};

struct ID_EX
{
    int alu_input1, alu_input2, rd;
    int32_t imm = 0;

    bool branch = false;
    int rs2;
    int alu_signal;
    bool rs1_needs_EX_to_EX = false;  // Needed in execute stage
    bool rs2_needs_EX_to_EX = false;  // needed in execute stage
    bool rs1_needs_MEM_to_EX = false; // needed in execute stage
    bool rs2_needs_MEM_to_EX = false; // needed in execute stage
    bool rs1_needs_MEM_to_MEM = false;
    bool rs2_needs_MEM_to_MEM = false;
    bool stall = false;               // needed in execute stage
    bool needs_mem = false;           // needed in memory stage
    bool write = false, read = false; // Memory signals
    int blocks;                       // lb,lh,lw,sb,sh,sw
    bool needs_writeback = true;
    int ra = 0;
    uint32_t pc;
    bool is_unconditional = false;
};

struct EX_MEM
{
    int rs2; // needed for store instructions
    int alu_output;
    int rd;
    bool needs_mem = false; // to be passed on from ID_EX
    bool write, read;
    bool needs_writeback = true; // needed in write back stage
    int blocks;
    bool rs1_needs_MEM_to_MEM = false;
    bool rs2_needs_MEM_to_MEM = false;
    bool branch = false;
    int ra = 0;
    int32_t imm;
    bool flush = false;
    uint32_t pc;
    bool is_unconditional = false;
    bool is_branching = false;
};

bool stall = false;

struct MEM_WB
{
    int mem_output; // In case read from memory
    bool needs_writeback = true;
    int rd;
};

IF_ID defaultbuffer1;
ID_EX default_buffer2;
EX_MEM default_buffer3;
MEM_WB default_buffer4;

IF_ID buffer1;
ID_EX buffer2;
EX_MEM buffer3;
MEM_WB buffer4;

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
    int index = -1;
    bool dependent_rs1 = false, dependent_rs2 = false;
};

struct PipelineStage
{
    Instruction *instr = nullptr;
};

vector<PipelineStage> pipeline(5); // IF, ID, EX, MEM, WB

bool needsForwarding(Instruction &curr, Instruction &prev, string in)
{
    if (!knob2)
        return false;

    if (prev.rd == 0)
        return false;
    if (prev.op == "NOP" || prev.rd == -1)
        return false;
    if ((curr.rs1 == prev.rd && curr.needs_rs1_in == in))
    {
        curr.dependent_rs1 = true;
        return true;
    }

    if ((curr.rs2 == prev.rd && curr.needs_rs2_in == in))
    {
        curr.dependent_rs2 = true;
        return true;
    }
    return false;
}

bool loadUseHazard(const Instruction &curr, const Instruction &prev)
{
    // cout<<buffer3.read<<" "<<curr.rs1<<" "<<curr.rs2<<" "<<buffer3.rd<<endl;
    return (buffer3.read) &&
           ((curr.rs1 == buffer3.rd && curr.needs_rs1_in == "EX") || (curr.rs2 == buffer3.rd && curr.needs_rs2_in == "EX"));
}

unordered_map<string, int> operationMap = {
    {"ADD", 1}, {"ADDI", 2}, {"LB", 3}, {"LD", 4}, {"LH", 5}, {"LW", 6}, {"JALR", 7}, {"JAL", 8}, {"SB", 9}, {"SH", 10}, {"SD", 11}, {"SW", 12}, {"BEQ", 13}, {"BNE", 14}, {"BGE", 15}, {"BLT", 16}, {"AND", 17}, {"ANDI", 18}, {"OR", 19}, {"ORI", 20}, {"MUL", 21}, {"DIV", 22}, {"REM", 23}, {"XOR", 24}, {"SUB", 25}, {"SLL", 26}, {"SLT", 27}, {"SRL", 28}, {"SRA", 29}, {"LUI", 30}, {"AUIPC", 31}};

uint32_t ALU(int val1, int val2, int OP)
{
    cout << val1 << " " << val2 << " " << OP << endl;
    if (OP == 1 || OP == 2 || OP == 3 || OP == 4 || OP == 5 || OP == 6 || OP == 7 || OP == 8 || OP == 9 || OP == 10 || OP == 11 || OP == 12)
    {
        return val1 + val2;
    }
    else if (OP == 13)
    {

        return buffer3.is_branching = (val1 == val2);
    }
    else if (OP == 14)
    {

        return buffer3.is_branching = (val1 != val2);
    }
    else if (OP == 15)
        return buffer3.is_branching = val1 >= val2;
    else if (OP == 16)
        return buffer3.is_branching = val1 < val2;
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

int IAG()
{
    if (buffer3.is_unconditional)
        return global_pc += 4;
    if (branchPrediction)
    {
        cout << "\033[1;32m## BRANCH PREDICTION ##\033[0m" << endl;
        return global_pc = predictedPc;
    }
    else if (!buffer3.branch && (global_pc / 4 < InstructionPCPairs.size()))
    {
        // cout << "IAG Call; new PC: 0x" <<hex<< global_pc + 4 <<dec<< endl;
        return global_pc = global_pc + 4;
    }
    else if (buffer3.ra)
    {
        cout << "JALR" << endl;
        // cout << "IAG Call; new PC: " << RegFile[buffer3.ra] + buffer3.imm << endl;
        buffer3.flush = true;
        return global_pc = RegFile[buffer3.ra] + buffer3.imm;
    }

    else if (buffer3.branch) // first time encountering branch
    {
        cout << "NORMAL BRANCHES " << endl;
        if (buffer3.is_branching)
            buffer3.flush = true;
        // cout << "IAG Call; new PC: " << global_pc + buffer3.imm << endl;
        return global_pc += 4;
    }
    else
    {
        return global_pc;
    }
}

Instruction decodeRType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    // cout<<&operationMap<<endl;
    // cout<<operationMap["SUB"]<<" "<<operationMap["OR"]<<" "<<operationMap["BEQ"]<<endl;
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
        buffer2.alu_signal = operationMap[curr.op];
        // cout << "Operation " << curr.op << " " << buffer2.alu_signal << endl;
        curr.rd = regNums[rd];
        curr.rs1 = regNums[rs1];
        curr.rs2 = regNums[rs2];
        curr.imm = 0;
        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = RegFile[curr.rs2];
        buffer2.rd = curr.rd;
        buffer2.rs2 = curr.rs2;
        buffer2.branch = false;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            buffer2.stall = true;
            ++statDataHazards;
            ++statStalls;
            ++statStallsData;
        }
        // Execute from previous instruction
        else if (knob2)
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_EX_to_EX = true : buffer2.rs2_needs_EX_to_EX = true;
                curr.dependent_rs1 ? curr.dependent_rs1 = false : curr.dependent_rs2 = false;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_MEM = true : buffer2.rs2_needs_MEM_to_MEM = true;
                curr.dependent_rs1 ? curr.dependent_rs1 = false : curr.dependent_rs2 = false;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_EX = true : buffer2.rs2_needs_MEM_to_EX = true;
                curr.dependent_rs1 ? curr.dependent_rs1 = false : curr.dependent_rs2 = false;
            }
        }
        else
        { // if data forwarding is disabled
            if (pipeline[2].instr)
            {
                Instruction &prev = *pipeline[2].instr;
                if ((curr.rs1 == prev.rd && curr.rs1 != 0 && curr.needs_rs1_in == "EX") ||
                    (curr.rs2 == prev.rd && curr.rs2 != 0 && curr.needs_rs2_in == "EX"))
                {
                    buffer2.stall = true;
                    ++statDataHazards;
                    ++statStalls;
                    ++statStallsData;
                }
            }
        }
        if (buffer2.rs1_needs_EX_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input1 = buffer3.alu_output;
        }
        else if (buffer2.rs1_needs_MEM_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input1 = buffer4.mem_output;
        }

        if (buffer2.rs2_needs_EX_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input2 = buffer3.alu_output;
        }
        else if (buffer2.rs2_needs_MEM_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input2 = buffer4.mem_output;
        }

        return curr;
    }
    return {instruction, "Unknown"};
}

Instruction decodeIType(uint32_t instruction, vector<PipelineStage> &pipeline)
{
    // cout<<&operationMap<<endl;
    // cout<<operationMap["SUB"]<<" "<<operationMap["OR"]<<" "<<operationMap["BEQ"]<<endl;
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
        buffer2.alu_signal = operationMap[curr.op];
        // cout << "Operation " << curr.op << " " << buffer2.alu_signal << endl;
        curr.rd = regNums[rd];
        buffer2.rd = curr.rd;
        curr.rs1 = regNums[rs1];
        curr.rs2 = -1;
        buffer2.rs2 = curr.rs2;
        curr.imm = imm;
        buffer2.stall = false;
        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = curr.imm;
        buffer2.branch = false;
        if (opcode == 0x03)
        {
            buffer2.needs_mem = true;
            buffer2.read = true;
            buffer2.write = false;
            buffer2.blocks = 1 << funct3;
        }
        else if (opcode == 0x67)
        {
            buffer2.branch = true;
            global_pc = RegFile[curr.rs1] + curr.imm;
            buffer2.ra = curr.rs1;
            buffer2.alu_input1 = global_pc;
            buffer2.alu_input2 = 4;
            buffer2.is_unconditional = true;
        }
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            buffer2.stall = true;

            ++statDataHazards;
            ++statStalls;
            ++statStallsData;
        }
        // Execute from previous instruction
        else if (knob2)
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_EX_to_EX = true : buffer2.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_MEM = true : buffer2.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_EX = true : buffer2.rs2_needs_MEM_to_EX = true;
            }
        }
        else
        { // if data forwArding is disabled
            if (pipeline[2].instr)
            {
                Instruction &prev = *pipeline[2].instr;
                if ((curr.rs1 == prev.rd && curr.rs1 != 0 && curr.needs_rs1_in == "EX") ||
                    (curr.rs2 == prev.rd && curr.rs2 != 0 && curr.needs_rs2_in == "EX"))
                {
                    buffer2.stall = true;
                    ++statDataHazards;
                    ++statStalls;
                    ++statStallsData;
                }
            }
        }
        if (buffer2.rs1_needs_EX_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input1 = buffer3.alu_output;
        }
        else if (buffer2.rs1_needs_MEM_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input1 = buffer4.mem_output;
        }

        if (buffer2.rs2_needs_EX_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input2 = buffer3.alu_output;
        }
        else if (buffer2.rs2_needs_MEM_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input2 = buffer4.mem_output;
        }
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
    buffer2.rs2 = rs2;
    uint32_t imm11_5 = (instruction >> 25) & 0x7F;
    RM = RegFile[rs2];
    int32_t imm = (imm11_5 << 5) | imm4_0;
    if (imm & (1 << 11))
        imm |= 0xFFFFF000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    Instruction curr;

    if (sTypeInstructions.find(key) != sTypeInstructions.end())
    {
        buffer2.needs_writeback = false;
        buffer2.needs_mem = true;
        buffer2.read = false;
        buffer2.write = true;
        buffer2.branch = false;
        buffer2.blocks = 1 << funct3;
        curr = {instruction, "EX", "MEM", sTypeInstructions[key], -1, regNums[rs1], regNums[rs2], imm};
        buffer2.alu_signal = operationMap[curr.op];

        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = curr.imm;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            buffer2.stall = true;

            ++statDataHazards;
            ++statStalls;
            ++statStallsData;
        }
        // Execute from previous instruction
        else if (knob2)
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_EX_to_EX = true : buffer2.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_MEM = true : buffer2.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_EX = true : buffer2.rs2_needs_MEM_to_EX = true;
            }
        }
        else
        { // if data forwarding is disabled
            if (pipeline[2].instr)
            {
                Instruction &prev = *pipeline[2].instr;
                if ((curr.rs1 == prev.rd && curr.rs1 != 0 && curr.needs_rs1_in == "EX") ||
                    (curr.rs2 == prev.rd && curr.rs2 != 0 && curr.needs_rs2_in == "EX"))
                {
                    buffer2.stall = true;
                    ++statDataHazards;
                    ++statStalls;
                    ++statStallsData;
                }
            }
        }
        if (buffer2.rs1_needs_EX_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input1 = buffer3.alu_output;
        }
        else if (buffer2.rs1_needs_MEM_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input1 = buffer4.mem_output;
        }

        if (buffer2.rs2_needs_EX_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input2 = buffer3.alu_output;
        }
        else if (buffer2.rs2_needs_MEM_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input2 = buffer4.mem_output;
        }
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
    buffer2.rs2 = rs2;
    uint32_t imm10_5 = (instruction >> 25) & 0x3F;
    uint32_t imm12 = (instruction >> 31) & 0x1;
    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    if (imm & (1 << 12))
        imm |= 0xFFFFE000;
    buffer2.imm = imm;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    Instruction curr;
    if (sbTypeInstructions.find(key) != sbTypeInstructions.end())
    {
        buffer2.branch = true;
        buffer2.needs_writeback = false;
        curr = {instruction, "EX", "EX", sbTypeInstructions[key], -1, regNums[rs1], regNums[rs2], imm};
        buffer2.alu_signal = operationMap[curr.op];

        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = RegFile[curr.rs2];
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {
            buffer2.stall = true;
            ++statDataHazards;
            ++statStalls;
            ++statStallsData;
        }
        // Execute from previous instruction
        else if (knob2)
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_EX_to_EX = true : buffer2.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_MEM = true : buffer2.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_EX = true : buffer2.rs2_needs_MEM_to_EX = true;
            }
        }
        else
        { // if data forwarding is disabled
            if (pipeline[2].instr)
            {
                Instruction &prev = *pipeline[2].instr;
                if ((curr.rs1 == prev.rd && curr.rs1 != 0 && curr.needs_rs1_in == "EX") ||
                    (curr.rs2 == prev.rd && curr.rs2 != 0 && curr.needs_rs2_in == "EX"))
                {
                    buffer2.stall = true;
                    ++statDataHazards;
                    ++statStalls;
                    ++statStallsData;
                }
            }
        }
        if (buffer2.rs1_needs_EX_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input1 = buffer3.alu_output;
        }
        else if (buffer2.rs1_needs_MEM_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input1 = buffer4.mem_output;
        }

        if (buffer2.rs2_needs_EX_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input2 = buffer3.alu_output;
        }
        else if (buffer2.rs2_needs_MEM_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input2 = buffer4.mem_output;
        }
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
        buffer2.rs2 = curr.rs2;
        buffer2.alu_signal = operationMap[curr.op];
        buffer2.branch = false;
        buffer2.alu_input1 = curr.imm;
        buffer2.alu_input2 = buffer1.pc;
        buffer2.rd = rd;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            buffer2.stall = true;
            ++statDataHazards;
            ++statStalls;
            ++statStallsData;
        }
        // Execute from previous instruction
        else if (knob2)
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_EX_to_EX = true : buffer2.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_MEM = true : buffer2.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_EX = true : buffer2.rs2_needs_MEM_to_EX = true;
            }
        }
        else
        { // if data forwarding is disabled
            if (pipeline[2].instr)
            {
                Instruction &prev = *pipeline[2].instr;
                if ((curr.rs1 == prev.rd && curr.rs1 != 0 && curr.needs_rs1_in == "EX") ||
                    (curr.rs2 == prev.rd && curr.rs2 != 0 && curr.needs_rs2_in == "EX"))
                {
                    buffer2.stall = true;
                    ++statDataHazards;
                    ++statStalls;
                    ++statStallsData;
                }
            }
        }
        if (buffer2.rs1_needs_EX_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input1 = buffer3.alu_output;
        }
        else if (buffer2.rs1_needs_MEM_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input1 = buffer4.mem_output;
        }

        if (buffer2.rs2_needs_EX_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input2 = buffer3.alu_output;
        }
        else if (buffer2.rs2_needs_MEM_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input2 = buffer4.mem_output;
        }
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
        buffer2.branch = true;
        buffer2.imm = imm;
        curr = {instruction, "", "", ujTypeInstructions[key], regNums[rd], -1, -1, imm};
        buffer2.rs2 = curr.rs2;
        buffer2.alu_signal = operationMap[curr.op];
        buffer2.alu_input1 = global_pc;
        buffer2.alu_input2 = 0;

        global_pc = buffer2.pc + curr.imm;
        buffer2.rd = rd;
        buffer2.is_unconditional = true;
        bool stall = false;
        if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr))
        {

            buffer2.stall = true;
            ++statDataHazards;
            ++statStalls;
            ++statStallsData;
        }
        // Execute from previous instruction
        else if (knob2)
        {
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_EX_to_EX = true : buffer2.rs2_needs_EX_to_EX = true;
            }
            // Memory from previous instruction
            if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr, "MEM"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_MEM = true : buffer2.rs2_needs_MEM_to_MEM = true;
            }
            // Execute from prev to prev instruction
            if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr, "EX"))
            {
                curr.dependent_rs1 ? buffer2.rs1_needs_MEM_to_EX = true : buffer2.rs2_needs_MEM_to_EX = true;
            }
        }
        else
        { // if data forwarding is disabled
            if (pipeline[2].instr)
            {
                Instruction &prev = *pipeline[2].instr;
                if ((curr.rs1 == prev.rd && curr.rs1 != 0 && curr.needs_rs1_in == "EX") ||
                    (curr.rs2 == prev.rd && curr.rs2 != 0 && curr.needs_rs2_in == "EX"))
                {
                    buffer2.stall = true;
                    ++statDataHazards;
                    ++statStalls;
                    ++statStallsData;
                }
            }
        }
        if (buffer2.rs1_needs_EX_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input1 = buffer3.alu_output;
        }
        else if (buffer2.rs1_needs_MEM_to_EX)
        {
            cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input1 = buffer4.mem_output;
        }

        if (buffer2.rs2_needs_EX_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
            buffer2.alu_input2 = buffer3.alu_output;
        }
        else if (buffer2.rs2_needs_MEM_to_EX)
        {
            cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
            buffer2.alu_input2 = buffer4.mem_output;
        }
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

    bool branch_inst = buffer2.branch;
    //   cout<<buffer2.alu_input1<<" "<<buffer2.alu_input2<<" "<<buffer2.alu_signal<<endl;
    int val = ALU(buffer2.alu_input1, buffer2.alu_input2, buffer2.alu_signal);
    buffer3.alu_output = val;

    // —— Branch‐outcome handling ——
    // buffer3.branch is true if the branch/jump should take
    if (branch_inst)
    {
        cout << "\tTaken/Not Taken: " << val << endl;
        cout << "\tPC inst: " << buffer2.pc << endl;
        uint32_t branchPC = buffer2.pc;
        uint32_t actualTarget;
        if (buffer2.ra)
        {
            cout << "JALR" << endl;
            cout << RegFile[buffer2.ra] << " " << buffer2.imm << endl;
            actualTarget = RegFile[buffer2.ra] + buffer2.imm;
        }
        else
        {
            actualTarget = branchPC + buffer2.imm;
        }

        cout << "\t ++ updating BTB & BHT ++ " << endl;

        auto it = BTB.find(buffer2.pc);

        if (it == BTB.end())
        {
            BTB[branchPC] = actualTarget;
            BHT[branchPC] = buffer3.is_branching;
            if (buffer3.is_branching)
            {
                cout << "HAD TO BRANCH BUT DIDN'T : FLUSHING NOW" << endl;
                pipeline[0].instr = nullptr;
                global_pc = actualTarget;
                stall = true;
                pipeline[1].instr = nullptr;
            }
        }
        else if (buffer2.is_unconditional)
        {
            BTB[branchPC] = actualTarget;
            BHT[branchPC] = buffer3.is_branching;

            pipeline[0].instr = nullptr;
            global_pc = actualTarget;
            stall = true;
            pipeline[1].instr = nullptr;
        }

        else
        {
            bool wasTaken = buffer3.is_branching;
            bool history = (BHT[branchPC]);
            cout << "\twasTaken: " << wasTaken << endl;
            cout << "\thistory: " << history << endl;
            if (history != wasTaken)
            {
                // flush IF & ID
                cout << "\033[1;35m\t-- Wrong Prediction --\033[0m" << endl;
                ++statBranchMispredicts;
                ++statControlHazards;
                ++statStalls;
                ++statStallsControl;
                pipeline[0].instr = nullptr;
                stall = true;
                pipeline[1].instr = nullptr;
                // correct PC
                global_pc = wasTaken
                                ? actualTarget
                                : (branchPC);
                cout << "\tactual pc: " << global_pc + 4 << endl;
                BHT[branchPC] = wasTaken;
            }
            else
            {
                cout << "\033[1;34m\t -- Correct Prediction -- \033[0m" << endl;
            }
        }
    }

    return val;
}

void MemAccess()
{
    if (buffer3.needs_mem)
    {
        if (buffer3.read)
        { // load instructions . buffer3.alu_output will be the effective address
            int32_t value = 0;
            for (int i = buffer3.blocks - 1; i >= 0; i--)
            {
                cout << buffer3.alu_output + i << " " << MainMemory[buffer3.alu_output + i] << endl;
                value = (value << 8) | MainMemory[buffer3.alu_output + i];
            }
            buffer4.mem_output = value;
            cout << " MEM OUTPUT " << value << endl;
        }
        else
        { // store, alu_output is effective address. buffer.rs2 is to be stored
            if (buffer3.rs2_needs_MEM_to_MEM)
                buffer3.rs2 = buffer4.mem_output;
            for (int i = 0; i < buffer3.blocks; i++)
            {

                MainMemory[buffer3.alu_output + i] = static_cast<uint8_t>(RegFile[buffer3.rs2] & 0xFF); // Extract lowest 8 bits
                buffer3.rs2 >>= 8;                                                                      // Shift right to get next byte
            }
        }
    }
    else
    {
        buffer4.mem_output = buffer3.alu_output;
    }
    return;
}

void WriteBack()
{
    // cout << buffer4.rd << " " << buffer4.mem_output << endl;
    if (buffer4.needs_writeback)
    {
        RegFile[buffer4.rd] = buffer4.mem_output;
    }
}

// ─── Single‑cycle execution routine for knob1 == false ───────────────────
void singleCycleExecution()
{
    // Walk through each instruction one at a time
    for (auto &p : InstructionPCPairs)
    {
        uint32_t pc = p.first;
        uint32_t mc = p.second;
        // —— FETCH —— (just grab mc)
        // —— DECODE ——
        vector<PipelineStage> dummy(5);
        Instruction instr = decodeInstruction(mc, dummy);
        // —— EXECUTE ——
        // we need to set up a minimal ID/EX buffer
        ID_EX localBUF = default_buffer2;
        localBUF.pc = pc;
        localBUF.alu_signal = operationMap[instr.op];
        localBUF.needs_mem = (instr.op == "LB" || instr.op == "LW" || instr.op == "SB" || instr.op == "SW");
        localBUF.read = (instr.op == "LB" || instr.op == "LW");
        localBUF.write = (instr.op == "SB" || instr.op == "SW");
        localBUF.blocks = (instr.op == "SB" ? 1 : (instr.op == "SW" ? 4 : (instr.op == "LB" ? 1 : 4)));
        // set operands
        localBUF.alu_input1 = RegFile[instr.rs1];
        localBUF.alu_input2 = (instr.needs_rs2_in == "" ? instr.imm : RegFile[instr.rs2]);
        int alu_out = ALU(localBUF.alu_input1, localBUF.alu_input2, localBUF.alu_signal);
        // —— MEMACCESS ——
        int mem_out = alu_out;
        if (localBUF.needs_mem)
        {
            if (localBUF.read)
            {
                // load
                mem_out = 0;
                for (int i = localBUF.blocks - 1; i >= 0; --i)
                    mem_out = (mem_out << 8) | MainMemory[alu_out + i];
            }
            else
            {
                // store
                int val = RegFile[instr.rs2];
                for (int i = 0; i < localBUF.blocks; ++i)
                {
                    MainMemory[alu_out + i] = val & 0xFF;
                    val >>= 8;
                }
            }
        }
        // —— WRITEBACK ——
        if (localBUF.read || instr.op != "SB" && instr.op != "SW")
        {
            RegFile[instr.rd] = mem_out;
        }
        // —— STATS ——
        ++statCycles;
        ++statInstCount;
        if (instr.op == "LB" || instr.op == "LW" || instr.op == "SB" || instr.op == "SW")
            ++statLoadsStores;
        else if (isBranchOpcode(mc & 0x7F))
            ++statControlInst;
        else
            ++statALUInst;
    }
    // write out stats file exactly as pipelined does:
    ofstream stats("phase3_stats.txt");
    stats << "Total cycles: " << statCycles << "\n"
          << "Instructions executed: " << statInstCount << "\n"
          << "CPI: " << fixed << setprecision(2)
          << (double)statCycles / statInstCount << "\n"
          << "Load/store instrs: " << statLoadsStores << "\n"
          << "ALU instrs: " << statALUInst << "\n"
          << "Control instrs: " << statControlInst << "\n"
          << "Total stalls/bubbles: 0\n" // single‑cycle has none
          << "Data hazards: 0\n"
          << "Stalls due to data hazards: 0\n"
          << "Control hazards: 0\n"
          << "Stalls due to control hazards: 0\n"
          << "Branch mispredictions: 0\n";
    stats.close();
    cout << "Single‑cycle simulation complete. Stats written to phase3_stats.txt\n";
}

int main()
{
    ifstream inputFile("output.mc"); // Open the input file containing machine code.
    if (!inputFile)
    {
        cerr << "Error: Unable to open input.mc" << endl;
        return 1;
    }

    // Clear the global instruction memory vector.
    InstructionPCPairs.clear();

    string line;
    uint32_t machineCode;
    int pc;

    // Read the file line by line and update both InstructionPCPairs and MainMemory.
    bool readingInstructions = true;
    while (getline(inputFile, line))
    {
        // Skip empty lines.
        if (line.empty())
            continue;

        // Check if we have reached the end of the instruction segment.
        if (line.find("#end of text segment") != string::npos)
        {
            readingInstructions = false;
            continue;
        }

        // When reading the instruction segment:
        if (readingInstructions)
        {
            // if line is "exit"
            //  if (to_uppercase(line) == "EXIT")
            //  {
            //      //emplace into the map the pc and 0x0
            //      InstructionPCPairs.emplace_back(pc, 0x0);
            //  }

            // Skip header lines (for example, those containing "Address")
            if (line.find("Address") != string::npos)
                continue;

            istringstream iss(line);
            string address, machineCodeStr;

            // Read the address and machine code from each line.
            if (!(iss >> address >> machineCodeStr))
                continue;
            // also read the instruction opname
            string opname;
            if (!(iss >> opname))
                continue;
            if (opname == "ld" || opname == "sd")
            {
                // throw stderror
                cerr << "Error: Unsupported instruction " << opname << " with respect to RV32" << endl;
                return 1;
            }
            // Remove trailing ':' from address if present.
            if (!address.empty() && address.back() == ':')
                address.pop_back();

            // Convert the address (PC) and machine code from hexadecimal to integers.
            pc = stoi(address, nullptr, 16);
            machineCode = stoul(machineCodeStr, nullptr, 16);

            // Save the pair (PC, machine code) to the instruction memory.
            InstructionPCPairs.emplace_back(pc, machineCode);
        }
        else
        {
            // Reading the data segment.
            // Skip header lines (for example, those containing "Memory")
            if (line.find("Memory") != string::npos)
                continue;

            istringstream iss(line);
            string memAddrStr, memValStr;

            if (!(iss >> memAddrStr >> memValStr))
                continue;

            // Convert memory address and value from hexadecimal to integers.
            uint32_t memAddr = stoi(memAddrStr, nullptr, 16);
            int memVal = stoul(memValStr, nullptr, 16);
            // cout<<"Memory address 0x"<<hex<<memAddr<<" has been loaded with value 0x"<<memVal<<endl;
            // Update MainMemory with the value at this address.
            MemAccessforDataSeg("SW", memVal, memAddr);
        }
    }
    inputFile.close();
    // If knob1 == false, run single‑cycle and exit
    if (!knob1)
    {
        singleCycleExecution();
        return 0;
    }

    Instruction nop; // represents a bubble (NOP)

    cout << "Pipeline simulation with forwarding and correct timing:\n\n";

    while (global_pc / 4 < InstructionPCPairs.size() || any_of(pipeline.begin(), pipeline.end(), [](auto &st)
                                                               { return st.instr != nullptr; }))
    {

        cout << "\033[1;31m============ Cycle " << ++statCycles << " ============\n\033[0m";

        stall = false;
        // ----------- STEP 1: Process Stages from WB to EX first -------------
        if (pipeline[4].instr)
        {
            cout << "\n**  WriteBack:  0x" << setfill('0') << setw(8) << hex << pipeline[4].instr->mc << dec << "\n";
            WriteBack();
            // ── Stat2 & categorize ────────────────────────────────────────
            ++statInstCount;
            string opc = pipeline[4].instr->op;
            if (opc == "LB" || opc == "LH" || opc == "LW" || opc == "SB" || opc == "SH" || opc == "SW")
                ++statLoadsStores;
            else if (isBranchOpcode(pipeline[4].instr->mc & 0x7F))
                ++statControlInst;
            else
                ++statALUInst;
            buffer4 = default_buffer4;
        }
        if (pipeline[3].instr)
        {
            cout << "\n**  MemAccess:  0x" << setfill('0') << setw(8) << hex << pipeline[3].instr->mc << dec << "\n";
            MemAccess();
            buffer4.rd = buffer3.rd;
            buffer4.needs_writeback = buffer3.needs_writeback;
            buffer3 = default_buffer3;
        }
        if (pipeline[2].instr)
        {
            cout << "\n**  Execute:    0x" << setfill('0') << setw(8) << hex << pipeline[2].instr->mc << dec << "\n";
            int val = Execute(); // This should read only current state (not updated by Decode)
            cout << "\tVAL " << val << endl;
        }
        buffer3.needs_mem = buffer2.needs_mem;
        buffer3.rd = buffer2.rd;
        buffer3.rs2 = buffer2.rs2;
        buffer3.rs1_needs_MEM_to_MEM = buffer2.rs1_needs_MEM_to_MEM;
        buffer3.rs2_needs_MEM_to_MEM = buffer2.rs2_needs_MEM_to_MEM;
        buffer3.read = buffer2.read;
        buffer3.write = buffer2.write;
        buffer3.blocks = buffer2.blocks;
        buffer3.needs_writeback = buffer2.needs_writeback;
        buffer3.ra = buffer2.ra;
        buffer3.imm = buffer2.imm;
        buffer3.branch = buffer2.branch;
        buffer3.pc = buffer2.pc;
        buffer3.is_unconditional = buffer2.is_unconditional;
        buffer2 = default_buffer2;

        // ----------- STEP 2: Hazard Detection & Decode -------------

        if (pipeline[1].instr) // ID stage
        {

            buffer2.pc = buffer1.pc;
            Instruction &curr = *pipeline[1].instr;
            cout << "\n**  Decode:     0x" << setfill('0') << setw(8) << hex << curr.mc << dec << "\n";
            uint32_t currPC = global_pc;
            // Decode the instruction and check for hazards
            Instruction decoded = decodeInstruction(curr.mc, pipeline);
            uint32_t opcode = curr.mc & 0x7F;
            curr = decoded;
            if (isBranchOpcode(opcode))
            {
                cout << "\tBranch instruction detected!" << endl;
            }
            stall = buffer2.stall;
        }
        cout << "\tfetch stall: " << stall << endl;
        cout << "\tglobal pc bool: " << (global_pc / 4 < InstructionPCPairs.size()) << endl;
        // ----------- STEP 3: Fetch Stage -------------
        if (!stall && global_pc / 4 < InstructionPCPairs.size())
        {
            auto it = BTB.find(global_pc);
            if (it != BTB.end())
            {
                cout << "\tBTB HIT!" << endl;
            }
            bool predictTaken = (it != BTB.end() && BHT[global_pc]);
            // print whether successful find or not - if not then populate BTB

            cout << "\n**  Fetch:      0x" << setfill('0') << setw(8) << hex << InstructionPCPairs[global_pc / 4].second << dec << "\n";
            pipeline[0].instr = new Instruction();
            pipeline[0].instr->mc = InstructionPCPairs[global_pc / 4].second;
            // NEW: Set the instruction index based on its order.
            pipeline[0].instr->index = global_pc / 4;
            buffer1.pc = global_pc;
            buffer1.instruction = pipeline[0].instr->mc;

            if (buffer3.flush)
            {
                cout << "FLUSHING AFTER FETCHING " << endl;
                pipeline[0].instr = nullptr;
                pipeline[1].instr = nullptr;
                buffer2 = default_buffer2;
            }
            if (predictTaken)
            {
                branchPrediction = true;
                predictedPc = it->second;
                // redirect fetch
            }
            else
            {
                branchPrediction = false;
            }
        }
        IAG();
        cout << "NEW PC IS: " << global_pc << endl;
        // ----------- STEP 4: Pipeline Register Update -------------
        if (stall)
        {
            // Insert bubble into EX, keep ID and IF stages
            pipeline[4] = pipeline[3];   // MEM → WB
            pipeline[3] = pipeline[2];   // EX → MEM
            pipeline[2].instr = nullptr; // EX becomes bubble

            // Revert PC if an instruction was fetched in this cycle

            delete pipeline[0].instr;
            pipeline[0].instr = nullptr;
            global_pc -= 4;

            cout << "STALLING THE PIPELINE!!! " << endl;
        }
        else
        {
            // Normal pipeline advancement
            pipeline[4] = pipeline[3];   // MEM → WB
            pipeline[3] = pipeline[2];   // EX → MEM
            pipeline[2] = pipeline[1];   // ID → EX
            pipeline[1] = pipeline[0];   // IF → ID
            pipeline[0].instr = nullptr; // Clear IF
        }

        cout << "\n";
        // ─── Point 6: Conditional Debug Prints ────────────────────────────────

        // knob3: dump full register file?
        if (knob3)
        {
            cout << "\n--- RegFile @ cycle " << statCycles << " ---\n";
            for (int i = 0; i < 32; ++i)
            {
                cout << "x" << i << ": " << RegFile[i] << "\n";
            }
        }

        // knob4: dump all pipeline‑register contents?
        // ─── Conditional Debug Prints for Knob4 and Knob5 ─────────────────────────────
        // ─── Conditional Debug Prints for Knob4 and Knob5 ─────────────────────────────
        if (knob4)
        {
            // If knob4 is enabled, print details for all pipeline stages.
            cout << "\n--- Pipeline registers @ cycle " << statCycles << " ---\n";
            cout << "IF/ID:    PC=0x" << hex << buffer1.pc
                 << "  instr=0x" << buffer1.instruction << dec << "\n";
            cout << "ID/EX:    PC=0x" << hex << buffer2.pc << dec
                 << "  alu_in1=" << buffer2.alu_input1
                 << "  alu_in2=" << buffer2.alu_input2
                 << "  rd=" << buffer2.rd
                 << "  imm=" << buffer2.imm
                 << "  branch=" << boolalpha << buffer2.branch << "\n";
            cout << "EX/MEM:   PC=0x" << hex << buffer3.pc << dec
                 << "  alu_out=" << buffer3.alu_output
                 << "  rd=" << buffer3.rd
                 << "  needs_mem=" << boolalpha << buffer3.needs_mem
                 << "  branch=" << boolalpha << buffer3.branch << "\n";
            cout << "MEM/WB:   rd=" << buffer4.rd
                 << "  mem_out=" << buffer4.mem_output
                 << "  writeback=" << boolalpha << buffer4.needs_writeback << "\n";
        }
        else if (knob5)
        {
            // If knob5 is enabled (and knob4 is disabled),
            // print details only for the pipeline stage(s) that contain the instruction being traced.
            bool printedSomething = false;
            cout << "\n--- Tracing instruction #" << knob5_num << " @ cycle " << statCycles << " ---\n";

            if (pipeline[0].instr && pipeline[0].instr->index == knob5_num)
            {
                cout << "IF/ID:    PC=0x" << hex << buffer1.pc
                     << "  instr=0x" << buffer1.instruction << dec << "\n";
                printedSomething = true;
            }
            if (pipeline[1].instr && pipeline[1].instr->index == knob5_num)
            {
                cout << "ID/EX:    PC=0x" << hex << buffer2.pc << dec
                     << "  alu_in1=" << buffer2.alu_input1
                     << "  alu_in2=" << buffer2.alu_input2
                     << "  rd=" << buffer2.rd
                     << "  imm=" << buffer2.imm
                     << "  branch=" << boolalpha << buffer2.branch << "\n";
                printedSomething = true;
            }
            if (pipeline[2].instr && pipeline[2].instr->index == knob5_num)
            {
                cout << "EX/MEM:   PC=0x" << hex << buffer3.pc << dec
                     << "  alu_out=" << buffer3.alu_output
                     << "  rd=" << buffer3.rd
                     << "  needs_mem=" << boolalpha << buffer3.needs_mem
                     << "  branch=" << boolalpha << buffer3.branch << "\n";
                printedSomething = true;
            }
            if (pipeline[3].instr && pipeline[3].instr->index == knob5_num)
            {
                cout << "MEM/WB:   rd=" << buffer4.rd
                     << "  mem_out=" << buffer4.mem_output
                     << "  writeback=" << boolalpha << buffer4.needs_writeback << "\n";
                printedSomething = true;
            }
            if (!printedSomething)
            {
                // Optionally, you can print a message if the traced instruction isn’t in any stage.
                cout << "Instruction #" << knob5_num << " is not currently in any pipeline stage.\n";
            }
        }

        if (knob6)
        {
            cout << "\n*** Branch Predictor Unit @ cycle " << statCycles << " ***\n";
            cout << " BTB entries:\n";
            for (auto &e : BTB)
            {
                cout << "   PC=0x" << hex << e.first
                     << "  -> target=0x" << e.second << dec
                     << "  hist=" << boolalpha << BHT[e.first] << nouppercase << "\n";
            }
            cout << dec;
        }
    }

    // Cleanup dynamically allocated instructions
    for (auto &stage : pipeline)
    {
        if (stage.instr != nullptr && stage.instr != nullptr)
        {
            delete stage.instr;
            stage.instr = nullptr;
        }
    }
    cout << "\033[1;36m+--------------------\033[0m" << endl;
    for (int i = 0; i < 31; i++)
        cout << i << " " << RegFile[i] << endl;
    cout << "\033[1;36m+--------------------\033[0m" << endl;
    // print BTB and BH
    cout << "\n"
         << endl;
    cout << "=== BTB === " << endl;
    for (auto it : BTB)
    {
        cout << "PC: 0x" << hex << it.first << " Target: 0x" << hex << it.second << dec << endl;
    }
    cout << "\n === BHT ===" << endl;
    for (auto it : BHT)
    {
        cout << "PC: 0x" << hex << it.first << dec << " Taken: " << it.second << endl;
    }

    cout << "\n === MAIN MEMORY ===" << endl;
    cout << hex;
    for (auto it : MainMemory)
    {
        cout << "0x" << it.first << " : 0x" << it.second << endl;
    }
    cout << dec;
    ofstream stats("phase3_stats.txt");
    stats << "Total cycles: " << statCycles << "\n";
    stats << "Instructions executed: " << statInstCount << "\n";
    stats << "CPI: " << fixed << setprecision(2)
          << (double)statCycles / statInstCount << "\n";
    stats << "Load/store instrs: " << statLoadsStores << "\n";
    stats << "ALU instrs: " << statALUInst << "\n";
    stats << "Control instrs: " << statControlInst << "\n";
    stats << "Total stalls/bubbles: " << statStalls << "\n";
    stats << "Data hazards: " << statDataHazards << "\n";
    stats << "Stalls due to data hazards: " << statStallsData << "\n";
    stats << "Control hazards: " << statControlHazards << "\n";
    stats << "Stalls due to control hazards: " << statStallsControl << "\n";
    stats << "Branch mispredictions: " << statBranchMispredicts << "\n";
    stats.close();
    cout << "Stats written to phase3_stats.txt\n";
    cout << "=== Simulation complete ===" << endl;
    return 0;
}
