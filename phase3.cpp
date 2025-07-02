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

bool knob1 = true;  // Knob1: Enable/disable pipelining
bool knob2 = true; // Knob2: Enable/disable data forwarding
bool knob3 = false; // Knob3: Print register file each cycle
bool knob4 = false; // Knob4: Print pipeline‑register info each cycle
bool knob5 = true;  // Knob5: Trace a specific instruction
int knob5_num = 1;  //      which instruction index to trace
bool knob6 = false; // Knob6: Print branch‑predictor details

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
vector<tuple<int, uint32_t, string>> InstructionPCPairs;
vector<int> RegFile = {0, 0, 2147483612, 268435456, 0, 0, 0, 0, 0, 0, 1, 2147483612, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
map<uint32_t, int> MainMemory;
bool IAG_CALLED;

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
void MemAccessforDataSeg(int size, int value, int eff)
{

    for (int i = 0; i < size; i++)
    {
        MainMemory[eff + i] = static_cast<uint8_t>(value & 0xFF); // Extract lowest 8 bits
        value >>= 8;                                              // Shift right to get next byte

        cout << "MainMemory[0x" << hex << eff + i << "] = " << MainMemory[eff + i] << dec << endl;
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
    bool flush = false;

    bool branch = false;
    int rs2;
    int alu_signal;
    bool rs1_needs_EX = false;  // Needed in execute stage
    bool rs2_needs_EX = false;  // needed in execute stage
    bool rs1_needs_MEM = false; // needed in execute stage
    bool rs2_needs_MEM = false; // needed in execute stage
    // bool rs1_needs_MEM_to_MEM = false;
    // bool rs2_needs_MEM_to_MEM = false;
    int stall = 0;                    // needed in execute stage
    bool needs_mem = false;           // needed in memory stage
    bool write = false, read = false; // Memory signals
    int blocks;                       // lb,lh,lw,sb,sh,sw
    bool needs_writeback = true;
    int ra = 0;
    uint32_t pc;
    bool is_unconditional = false;
    int rs1;
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

    bool branch = false;
    int ra = 0;
    int32_t imm;
    bool flush = false;
    uint32_t pc;
    bool is_unconditional = false;
    bool is_branching = false;
    int mem_output = 0; // Required when forwarding from mem to mem
    int alu_input2;     // Needed for store instructions
};

int stall = 0;

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

    string op;

    int rd;
    int rs1;
    int rs2 = -1;
    int32_t imm;
    int pc;
    int index = -1;
};

struct PipelineStage
{
    Instruction *instr = nullptr;
};

vector<PipelineStage> pipeline(5); // IF, ID, EX, MEM, WB

void ForwardingUnit()
{

    if (buffer3.needs_writeback && buffer3.rd != 0)
    {
        if (buffer3.rd == buffer2.rs1)
            buffer2.rs1_needs_EX = true;
        if (buffer3.rd == buffer2.rs2)
            buffer2.rs2_needs_EX = true;
    }

    if (buffer4.needs_writeback && buffer4.rd != 0)
    {
        if (buffer4.rd == buffer2.rs1)
            buffer2.rs1_needs_MEM = true;
        if (buffer4.rd == buffer2.rs2)
            buffer2.rs2_needs_MEM = true;
    }
}

int HDU()
{

    if (knob2)
    {
        if ((buffer3.read && (buffer2.rs1 == buffer3.rd || buffer2.rs2 == buffer3.rd)) && buffer3.rd != 0)
            return 1;
    }
    else
    {
        if ((buffer3.rd == buffer2.rs1 || buffer3.rd == buffer2.rs2) && buffer3.rd != 0)
            return 2;
        else if ((buffer4.rd == buffer2.rs1 || buffer4.rd == buffer2.rs2) && buffer4.rd != 0)
            return 1;
    }
    return 0;
}
unordered_map<string, int> operationMap = {
    {"ADD", 1}, {"ADDI", 2}, {"LB", 3}, {"LD", 4}, {"LH", 5}, {"LW", 6}, {"JALR", 7}, {"JAL", 8}, {"SB", 9}, {"SH", 10}, {"SD", 11}, {"SW", 12}, {"BEQ", 13}, {"BNE", 14}, {"BGE", 15}, {"BLT", 16}, {"AND", 17}, {"ANDI", 18}, {"OR", 19}, {"ORI", 20}, {"MUL", 21}, {"DIV", 22}, {"REM", 23}, {"XOR", 24}, {"SUB", 25}, {"SLL", 26}, {"SLT", 27}, {"SRL", 28}, {"SRA", 29}, {"LUI", 30}, {"AUIPC", 31}};

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

int IAG(uint32_t curr_pc, bool usual = false)
{
    if (!usual)
    {
        // if present in BTB
        if (branchPrediction)
        {
            cout << "CURRENT_PC : 0x" << hex << curr_pc << endl;
            cout << "\033[1;32m## BRANCH PREDICTION ##\033[0m" << endl;
            return global_pc = BHT[curr_pc] == 1 ? BTB[curr_pc] : curr_pc + 4;
        }
        if (stall)
        {
            return global_pc;
        }
        else if (buffer2.ra)
        {
            cout << "JALR" << endl;
            // cout << "IAG Call; new PC: " << RegFile[buffer3.ra] + buffer3.imm << endl;
            cout << "buffer2.ra: " << buffer2.ra << endl;
            cout << "buffer2.imm: " << buffer2.imm << endl;
            cout << "RegFile[buffer2.ra]: " << RegFile[buffer2.ra] << endl;
            return global_pc = RegFile[buffer2.ra] + buffer2.imm;
        }

        if (buffer2.is_unconditional)
        {
            cout << "JAL ENCOUNTERED" << endl;
            return global_pc = curr_pc + buffer2.imm;
        }

        if (buffer3.is_branching)
        {
            cout << "BUFFER3 IS BRANCHING" << endl;
            return global_pc = curr_pc + buffer2.imm;
        }

        else
        {
            cout << "Error corrected: " << curr_pc + 4 << endl;
            // std::cout << "Pausing for 3 seconds...\n";
            // std::this_thread::sleep_for(std::chrono::seconds(3));
            return global_pc = curr_pc + 4;
        }
    }
    else
    {
        //cout << global_pc << " " << InstructionPCPairs.size() << endl;
        if ((global_pc / 4 < InstructionPCPairs.size()))
        {
            cout << "NORMAL IAG CALL" << endl;
            // cout << "IAG Call; new PC: 0x" <<hex<< global_pc + 4 <<dec<< endl;
            return global_pc = global_pc + 4;
        }
        else
        {
            return global_pc;
        }
    }
    // if(!buffer3.is_branching){
    //     cout<<"BUFFER3 IS NOT BRANCHING"<<endl;
    //     return global_pc = curr_pc+4;}

    // else if(buffer3.branch)  //first time encountering branch
    // {
    //     cout<<"NORMAL BRANCHES "<<endl;
    //    if(buffer3.is_branching)buffer3.flush = true;
    //     // cout << "IAG Call; new PC: " << global_pc + buffer3.imm << endl;
    //     return global_pc +=4;
    // }
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

        curr.op = rTypeInstructions[key];
        buffer2.alu_signal = operationMap[curr.op];
        // cout << "Operation " << curr.op << " " << buffer2.alu_signal << endl;
        curr.rd = regNums[rd];
        curr.rs1 = regNums[rs1];
        buffer2.rs1 = curr.rs1;
        curr.rs2 = regNums[rs2];
        curr.imm = 0;
        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = RegFile[curr.rs2];
        buffer2.rd = curr.rd;
        buffer2.rs2 = curr.rs2;
        buffer2.branch = false;
        buffer2.stall = HDU();
        if (pipeline[2].instr)
        {

            ++statDataHazards;
            statStalls += buffer2.stall;
            ++statStallsData;
        }
        // Execute from previous instruction

        ForwardingUnit();

        if (knob2)
        {
            if (buffer2.rs1_needs_EX)
            {

                cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input1 = buffer3.alu_output;
            }
            else if (buffer2.rs1_needs_MEM)
            {
                cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input1 = buffer4.mem_output;
            }

            if (buffer2.rs2_needs_EX)
            {
                cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input2 = buffer3.alu_output;
            }
            else if (buffer2.rs2_needs_MEM)
            {
                cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input2 = buffer4.mem_output;
            }
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

        curr.op = iTypeInstructions[key];
        buffer2.alu_signal = operationMap[curr.op];
        // cout << "Operation " << curr.op << " " << buffer2.alu_signal << endl;
        curr.rd = regNums[rd];
        buffer2.rd = curr.rd;
        curr.rs1 = regNums[rs1];
        buffer2.rs1 = curr.rs1;
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
            buffer2.ra = curr.rs1;
            buffer2.alu_input1 = global_pc;
            buffer2.alu_input2 = 4;
            buffer2.is_unconditional = false; // x1 is varaible, can't predict same each time
            // buffer2.flush = true;
            // IAG_CALLED = true;
            cout << "JALR ENCOUNTERED" << endl;
            // IAG(buffer2.pc);
            // BTB[buffer2.pc] = global_pc;
            // BHT[buffer2.pc] = 1;
        }
        buffer2.stall = HDU();
        if (pipeline[2].instr)
        {

            ++statDataHazards;
            statStalls += buffer2.stall;
            ++statStallsData;
        }
        // Execute from previous instruction

        ForwardingUnit();

        if (knob2)
        {
            if (buffer2.rs1_needs_EX)
            {

                cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input1 = buffer3.alu_output;
            }
            else if (buffer2.rs1_needs_MEM)
            {
                cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input1 = buffer4.mem_output;
            }

            if (buffer2.rs2_needs_EX)
            {
                cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input2 = buffer3.alu_output;
            }
            else if (buffer2.rs2_needs_MEM)
            {
                cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input2 = buffer4.mem_output;
            }
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
    cout << "RS2 : " << rs2 << endl;
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
        curr = {instruction, sTypeInstructions[key], -1, regNums[rs1], regNums[rs2], imm};
        buffer2.rs1 = curr.rs1;
        buffer2.alu_signal = operationMap[curr.op];

        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = curr.imm;
        cout << buffer2.alu_input1 << " " << buffer2.alu_input2 << " " << endl;
        buffer2.stall = HDU();
        if (pipeline[2].instr)
        {

            ++statDataHazards;
            statStalls += buffer2.stall;
            ++statStallsData;
        }
        // Execute from previous instruction

        ForwardingUnit();

        if (knob2)
        {
            if (buffer2.rs1_needs_EX)
            {

                cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input1 = buffer3.alu_output;
            }
            else if (buffer2.rs1_needs_MEM)
            {
                cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input1 = buffer4.mem_output;
            }
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
        curr = {instruction, sbTypeInstructions[key], -1, regNums[rs1], regNums[rs2], imm};
        buffer2.alu_signal = operationMap[curr.op];
        buffer2.rs1 = curr.rs1;

        buffer2.alu_input1 = RegFile[curr.rs1];
        buffer2.alu_input2 = RegFile[curr.rs2];
        buffer2.stall = HDU();
        if (pipeline[2].instr)
        {

            ++statDataHazards;
            statStalls += buffer2.stall;
            ++statStallsData;
        }
        // Execute from previous instruction

        ForwardingUnit();

        if (knob2)
        {
            if (buffer2.rs1_needs_EX)
            {

                cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input1 = buffer3.alu_output;
            }
            else if (buffer2.rs1_needs_MEM)
            {
                cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input1 = buffer4.mem_output;
            }

            if (buffer2.rs2_needs_EX)
            {
                cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input2 = buffer3.alu_output;
            }
            else if (buffer2.rs2_needs_MEM)
            {
                cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input2 = buffer4.mem_output;
            }
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
        curr = {instruction, uTypeInstructions[key], regNums[rd], -1, -1, imm};
        buffer2.rs2 = curr.rs2;
        buffer2.alu_signal = operationMap[curr.op];
        buffer2.branch = false;
        buffer2.alu_input1 = curr.imm;
        buffer2.alu_input2 = buffer1.pc;
        buffer2.rd = rd;
        buffer2.stall = HDU();
        if (pipeline[2].instr)
        {

            ++statDataHazards;
            statStalls += buffer2.stall;
            ++statStallsData;
        }
        // Execute from previous instruction

        ForwardingUnit();

        if (knob2)
        {
            if (buffer2.rs1_needs_EX)
            {

                cout << " RS1 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input1 = buffer3.alu_output;
            }
            else if (buffer2.rs1_needs_MEM)
            {
                cout << " RS1 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input1 = buffer4.mem_output;
            }

            if (buffer2.rs2_needs_EX)
            {
                cout << " RS2 NEEDS FORWARDING FROM EX" << endl;
                buffer2.alu_input2 = buffer3.alu_output;
            }
            else if (buffer2.rs2_needs_MEM)
            {
                cout << " RS2 NEEDS FORWARDING FROM MEM" << endl;
                buffer2.alu_input2 = buffer4.mem_output;
            }
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
        curr = {instruction, ujTypeInstructions[key], regNums[rd], -1, -1, imm};
        buffer2.rs2 = curr.rs2;

        buffer2.alu_signal = operationMap[curr.op];
        buffer2.alu_input1 = buffer2.pc;
        buffer2.alu_input2 = 4;

        buffer2.rd = rd;
        buffer2.is_unconditional = true;

        cout << "JAL ENCOUNTERED" << endl;
        auto it = BTB.find(buffer2.pc);
        if (it == BTB.end())
        {
            IAG_CALLED = true;
            IAG(buffer2.pc);
            BTB[buffer2.pc] = global_pc;
            BHT[buffer2.pc] = 1;
            buffer2.flush = true;
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

uint32_t ALU(int val1, int val2, int OP)
{

    cout << "ALU OP: " << OP << " " << "val1: " << val1 << " val2: " << val2 << endl;
    if (OP == 1 || OP == 2 || OP == 3 || OP == 4 || OP == 5 || OP == 6 || OP == 7 || OP == 8 || OP == 9 || OP == 10 || OP == 11 || OP == 12)
    {
        return val1 + val2;
    }
    else if (OP == 13)
    {

        return (val1 == val2);
    }
    else if (OP == 14)
    {

        return (val1 != val2);
    }
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

void BPU(int val)
{

    auto it = BTB.find(global_pc);

    if (it != BTB.end())
    {
        branchPrediction = true;

        bool history = (BHT[global_pc]);
        cout << "\thistory: " << history << endl;
        IAG_CALLED = true;
        IAG(global_pc);
    }
}

int Execute()
{
    uint32_t current_pc = buffer2.pc;
    bool branch_inst = buffer2.branch;

    int val = ALU(buffer2.alu_input1, buffer2.alu_input2, buffer2.alu_signal);
    buffer3.alu_output = val;

    if (branch_inst && !buffer2.is_unconditional)
    {
        if (buffer2.ra)
        {
            buffer3.is_branching = true; // in case of jalr, val is ra
            auto it = BTB.find(current_pc);
            if (it != BTB.end())
            { // it was there before. thus a prediction was made.
                if (BTB[current_pc] != RegFile[buffer2.ra] + buffer2.imm)
                {
                    cout << "\033[1;35m\t-- Wrong Prediction, Flushing Now. --\033[0m" << endl;
                    ++statBranchMispredicts;
                    ++statControlHazards;
                    ++statStalls;
                    ++statStallsControl;
                    buffer3.flush = true;
                    branchPrediction = false;
                    IAG_CALLED = true;
                    cout << "wrong prediction in execute" << endl;
                    cout << current_pc << endl;
                    IAG(current_pc); // called again to get the correct address
                }
                else
                {
                    cout << "\033[1;34m\t -- Correct Prediction -- \033[0m" << endl;
                    cout << "PREDICTED : " << BHT[current_pc] << " ACTUAL : " << val << endl;
                }
            }
            else
            { // First time seeing the conditional branch instruction
                cout << "FIRST TIME ENCOUNTERING BRANCH" << endl;
                cout << "buffer2.ra: " << buffer2.ra << endl;
                cout << "buffer2.imm: " << buffer2.imm << endl;
                cout << "regfile[buffer2.ra]: " << RegFile[buffer2.ra] << endl;
                BTB[current_pc] = RegFile[buffer2.ra] + buffer2.imm;
                cout << BTB[current_pc] << endl;
                cout << "buffer3.is_branching: " << buffer3.is_branching << endl;
                cout << "global pc : " << global_pc << endl;
                if (buffer3.is_branching)
                {
                    IAG_CALLED = true;
                    cout << "is branching in execute" << endl;
                    IAG(current_pc);
                    buffer3.flush = true;
                    pipeline[1].instr = nullptr;
                }
            }
            BHT[current_pc] = 1;
        }
        else
        {
            buffer3.is_branching = val; // in case of jalr, val is ra
            auto it = BTB.find(current_pc);
            if (it != BTB.end())
            { // it was there before. thus a prediction was made.
                if (BHT[current_pc] != val)
                {
                    cout << "\033[1;35m\t-- Wrong Prediction, Flushing Now. --\033[0m" << endl;
                    ++statBranchMispredicts;
                    ++statControlHazards;
                    ++statStalls;
                    ++statStallsControl;
                    buffer3.flush = true;
                    branchPrediction = false;
                    IAG_CALLED = true;
                    cout << "wrong prediction in execute" << endl;
                    cout << "Current PC : 0x" << hex << current_pc << dec << endl;
                    IAG(current_pc); // called again to get the correct address
                }
                else
                {
                    cout << "\033[1;34m\t -- Correct Prediction -- \033[0m" << endl;
                    cout << "PREDICTED : " << BHT[current_pc] << " ACTUAL : " << val << endl;
                }
            }
            else
            { // First time seeing the conditional branch instruction
                cout << "FIRST TIME ENCOUNTERING BRANCH" << endl;
                BTB[current_pc] = current_pc + buffer2.imm;
                cout << BTB[current_pc] << endl;
                cout << "buffer3.is_branching: " << buffer3.is_branching << endl;
                cout << "global pc : " << global_pc << endl;
                if (buffer3.is_branching)
                {
                    IAG_CALLED = true;
                    cout << "is branching in execute" << endl;
                    IAG(current_pc);
                    buffer3.flush = true;
                    pipeline[1].instr = nullptr;
                }
            }
            BHT[current_pc] = val;
        }
    }

    return val;
}

void MemAccess()
{
    if (buffer3.needs_mem)
    {
        if (buffer3.read)
        {
            // Load instruction: buffer3.alu_output is the effective address
            int value = 0;

            for (int i = buffer3.blocks - 1; i >= 0; i--)
            {
                cout << buffer3.alu_output + i << " " << MainMemory[buffer3.alu_output + i] << endl;
                value = (value << 8) | MainMemory[buffer3.alu_output + i];
            }

            // Apply sign extension depending on how many bytes were loaded
            if (buffer3.blocks == 1) // lb
            {
                if (value & 0x80) // sign bit for 8-bit
                    value |= 0xFFFFFF00;
            }
            else if (buffer3.blocks == 2) // lh
            {
                if (value & 0x8000) // sign bit for 16-bit
                    value |= 0xFFFF0000;
            }
            // else: lw (4 bytes), no need to sign-extend

            buffer4.mem_output = value;
            cout << " MEM OUTPUT " << value << endl;
        }

        else
        { // store, alu_output is effective address. buffer.rs2 is to be stored

            int store = RegFile[buffer3.rs2];

            for (int i = 0; i < buffer3.blocks; i++)
            {
                cout << "buffer3.rs2: " << buffer3.rs2 << endl;
                cout << "RegFile[buffer3.rs2] " << RegFile[buffer3.rs2] << endl;
                MainMemory[buffer3.alu_output + i] = static_cast<uint8_t>(store & 0xFF); // Extract lowest 8 bits
                store >>= 8;                                                             // Shift right to get next byte
                cout << "Writing to memory address: 0x" << hex << (buffer3.alu_output + i) << " value: 0x" << MainMemory[buffer3.alu_output + i] << dec << endl;
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
    if (buffer4.needs_writeback && buffer4.rd != 0)
    {
        RegFile[buffer4.rd] = buffer4.mem_output;
        cout << "Reg[bufer4.rd]: " << RegFile[buffer4.rd] << " " << buffer4.rd << endl;
    }
}

void singleCycleExecution(int time)
{
    // Walk through each instruction one at a time
    for (auto &p : InstructionPCPairs)
    {
        uint32_t pc = get<0>(p);
        uint32_t mc = get<1>(p);
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
        localBUF.alu_input2 = (instr.rs2 == -1 ? instr.imm : RegFile[instr.rs2]);
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

        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
    // print memory
    cout << "Final Memory State:" << endl;
    for (auto pair:MainMemory)
    {
        uint32_t i = pair.first;
        uint32_t value = pair.second;
        cout << "Address: 0x" << hex << i << " Value: 0x" << value << dec << endl;
    }
    //print registers
    cout << "Final Register State:" << endl;
    for (int i = 0; i < 32; ++i)
    {
        cout << "Reg[" << i << "] = 0x" <<hex<< RegFile[i] <<dec<< endl;
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
    int time;
    cout << "Enter time per cycle (in ms): " << endl;
    cin >> time;
    cout << "Mode of execution (Run or Step): " << endl;
    int step = 0;
    cin >> step;
    // take input for all 6 knobs
    //  cout << "Enter knob1 (0 or 1): " << endl;
    //  cin >> knob1;
    //  cout << "Enter knob2 (0 or 1): " << endl;
    //  cin >> knob2;
    //  cout << "Enter knob3 (0 or 1): " << endl;
    //  cin >> knob3;
    //  cout << "Enter knob4 (0 or 1): " << endl;
    //  cin >> knob4;
    //  cout << "Enter knob5 (0 or 1): " << endl;
    //  cin >> knob5;
    //  //if knob5 == 1, take input for knob5_num
    //  if (knob5 == 1)
    //  {
    //      cout << "Enter knob5_num: " << endl;
    //      cin >> knob5_num;
    //  }
    //  cout << "Enter knob6 (0 or 1): " << endl;
    //  cin >> knob6;
    ifstream inputFile("input.mc"); // Open the input file containing machine code.
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

            // Also read the full instruction (up to but not including "#")
            string token, instructionStr;
            while (iss >> token)
            {
                if (token == "#")
                    break;
                if (instructionStr.empty())
                    instructionStr = token;
                else
                    instructionStr += " " + token;
            }

            // Extract only the opcode (first word of instruction)
            istringstream instSS(instructionStr);
            string opname;
            instSS >> opname;

            if (opname == "ld" || opname == "sd")
            {
                cerr << "Error: Unsupported instruction " << opname << " with respect to RV32" << endl;
                return 1;
            }

            // Remove trailing ':' from address if present.
            if (!address.empty() && address.back() == ':')
                address.pop_back();

            // Convert the address (PC) and machine code from hexadecimal to integers.
            int pc = stoi(address, nullptr, 16);
            uint32_t machineCode = stoul(machineCodeStr, nullptr, 16);

            cout << "PC: 0x" << hex << pc << " Machine Code: 0x" << machineCode
                 << " Instruction: " << instructionStr << endl;

            // Save the pair (PC, machine code) to the instruction memory.
            InstructionPCPairs.emplace_back(pc, machineCode, instructionStr);
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
            int size;

            // cout<<"Memory address 0x"<<hex<<memAddr<<" has been loaded with value 0x"<<memVal<<endl;
            // Update MainMemory with the value at this address.
            MemAccessforDataSeg(4, memVal, memAddr);
        }
    }
    inputFile.close();
    // If knob1 == false, run single‑cycle and exit
    if (!knob1)
    {
        singleCycleExecution(time);
        return 0;
    }

    Instruction nop; // represents a bubble (NOP)

    cout << "Pipeline simulation with forwarding and correct timing:\n\n";

    while (global_pc / 4 < InstructionPCPairs.size() || any_of(pipeline.begin(), pipeline.end(), [](auto &st)
                                                               { return st.instr != nullptr; }))
    {
        IAG_CALLED = false;
        branchPrediction = false;
        cout << "\033[1;31m============ Cycle " << ++statCycles << " ============\n\033[0m";
        stall = false;
        uint32_t curr_pc = global_pc;
        // ----------- STEP 1: Process Stages from WB to EX first -------------
        if (pipeline[4].instr)
        {
            cout << "\n**  WriteBack:  0x" << setfill('0') << setw(8) << hex << pipeline[4].instr->mc << " #" << get<2>(InstructionPCPairs[(pipeline[4].instr->pc) / 4]) << "#" << "\n";

            cout << "PC: 0x" <<hex<< pipeline[4].instr->pc <<dec<<endl;
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
            cout << "\n**  MemAccess:  0x" << setfill('0') << setw(8) << hex << pipeline[3].instr->mc << " #" << get<2>(InstructionPCPairs[(pipeline[3].instr->pc) / 4]) << "#" << "\n";

            cout << "PC: 0x" <<hex<< pipeline[3].instr->pc <<dec<< endl;
            MemAccess();
            buffer4.rd = buffer3.rd;
            buffer4.needs_writeback = buffer3.needs_writeback;

            buffer3 = default_buffer3;
        }
        if (pipeline[2].instr)
        {
            cout << "\n**  Execute:    0x" << setfill('0') << setw(8) << hex << pipeline[2].instr->mc << " #" << get<2>(InstructionPCPairs[(pipeline[2].instr->pc) / 4]) << "#" << "\n";
            cout << "PC: 0x" <<hex<< pipeline[2].instr->pc << dec<<endl;
            int val = Execute(); // This should read only current state (not updated by Decode)
            cout << "\tVAL " << val << endl;
            cout << hex << endl;

            buffer3.needs_mem = buffer2.needs_mem;
            buffer3.rd = buffer2.rd;
            buffer3.rs2 = buffer2.rs2;

            buffer3.read = buffer2.read;
            buffer3.write = buffer2.write;
            buffer3.blocks = buffer2.blocks;
            buffer3.needs_writeback = buffer2.needs_writeback;
            buffer3.ra = buffer2.ra;
            buffer3.imm = buffer2.imm;
            buffer3.branch = buffer2.branch;
            buffer3.pc = buffer2.pc;
            buffer3.is_unconditional = buffer2.is_unconditional;
            buffer3.alu_input2 = buffer2.alu_input2;
        }

        buffer2 = default_buffer2;

        if (buffer3.flush)
        {
            cout << "FLUSHING AFTER RESULT OF EXECUTE " << endl;
            pipeline[0].instr = nullptr;
            pipeline[1].instr = nullptr;
            buffer2 = default_buffer2;
        }

        // ----------- STEP 2: Hazard Detection & Decode -------------

        if (pipeline[1].instr) // ID stage
        {

            buffer2.pc = buffer1.pc;
            Instruction &curr = *pipeline[1].instr;
            cout << "\n**  Decode:     0x" << setfill('0') << setw(8) << hex << curr.mc << " #" << get<2>(InstructionPCPairs[buffer2.pc / 4]) << "#" << "\n";
            uint32_t currPC = global_pc;
            // Decode the instruction and check for hazards
            Instruction decoded = decodeInstruction(curr.mc, pipeline);
            decoded.pc = pipeline[1].instr->pc;
            uint32_t opcode = curr.mc & 0x7F;
            curr = decoded;
            if (isBranchOpcode(opcode))
            {
                cout << "\tBranch instruction detected!" << endl;
            }
            stall = buffer2.stall;
            // cout << "STALL : " << stall << endl;
        }

        if (buffer2.flush)
        {
            cout << "FLUSHING BECAUSE OF UNCONDITIONAL JUMP" << endl;
            pipeline[0].instr = nullptr;
        }
        cout << "buffer3.flush: " << buffer3.flush << endl;

        // ----------- STEP 3: Fetch Stage -------------
        if (!buffer2.flush && !buffer3.flush && stall == 0 && global_pc / 4 < InstructionPCPairs.size())
        {
            auto it = BTB.find(global_pc);
            if (it != BTB.end())
            {
                cout << "\tBTB HIT!" << endl;
            }

            // print whether successful find or not - if not then populate BTB

            cout << "\n**  Fetch:      0x" << setfill('0') << setw(8) << hex << get<1>(InstructionPCPairs[curr_pc / 4]) << " #" << get<2>(InstructionPCPairs[curr_pc / 4]) << "#" << "\n";

            pipeline[0].instr = new Instruction();
            pipeline[0].instr->mc = get<1>(InstructionPCPairs[curr_pc / 4]);
            pipeline[0].instr->pc = curr_pc;
            cout << "PC: 0x" <<hex<< curr_pc << " pipeline pc: 0x" << pipeline[0].instr->pc <<dec<< endl;
            buffer1.pc = curr_pc;
            buffer1.instruction = pipeline[0].instr->mc;

            auto i = BTB.find(curr_pc);
            if (i != BTB.end())
            {
                cout << "BRANCH FOUND IN BTB!. MAKING PREDICTION" << BHT[curr_pc] << endl;
                branchPrediction = true;
            }
        }

        // ----------- STEP 4: Pipeline Register Update -------------
        if (stall)
        {
            cout << "EXECUTE IS MADE NULL" << endl;
            // Insert bubble into EX, keep ID and IF stages
            pipeline[4] = pipeline[3];   // MEM → WB
            pipeline[3] = pipeline[2];   // EX → MEM
            pipeline[2].instr = nullptr; // EX becomes bubble

            // Revert PC if an instruction was fetched in this cycle

            delete pipeline[0].instr;
            pipeline[0].instr = nullptr;

            cout << "STALLING THE PIPELINE!!! " << endl;
        }
        else
        {
            if (knob5 && !knob4)
            {
                // If knob5 is enabled (and knob4 is disabled),
                // print details only for the pipeline stage(s) that contain the instruction being traced.
                bool printedSomething = false;
                cout << "\n--- Tracing instruction #" << knob5_num << " @ cycle " << statCycles << " ---\n";

                if (pipeline[0].instr && ((pipeline[0].instr->pc) / 4 + 1) == knob5_num)
                {
                    cout << "IF/ID:    PC=0x" << hex << buffer1.pc
                         << "  instr=0x" << buffer1.instruction << dec << "\n";
                    printedSomething = true;
                }
                if (pipeline[1].instr && ((pipeline[1].instr->pc) / 4 + 1) == knob5_num)
                {
                    cout << "ID/EX:    PC=0x" << hex << buffer2.pc << dec
                         << "  alu_in1=" << buffer2.alu_input1
                         << "  alu_in2=" << buffer2.alu_input2
                         << "  rd=" << buffer2.rd
                         << "  imm=" << buffer2.imm
                         << "  branch=" << boolalpha << buffer2.branch << "\n";
                    printedSomething = true;
                }
                if (pipeline[2].instr && ((pipeline[2].instr->pc) / 4 + 1) == knob5_num)
                {
                    cout << "EX/MEM:   PC=0x" << hex << buffer3.pc << dec
                         << "  alu_out=" << buffer3.alu_output
                         << "  rd=" << buffer3.rd
                         << "  needs_mem=" << boolalpha << buffer3.needs_mem
                         << "  branch=" << boolalpha << buffer3.branch << "\n";
                    printedSomething = true;
                }
                if (pipeline[3].instr && ((pipeline[3].instr->pc) / 4 + 1) == knob5_num)
                {
                    cout << "MEM/WB:   rd=" << buffer4.rd
                         << "  mem_out=" << buffer4.mem_output
                         << "  writeback=" << boolalpha << buffer4.needs_writeback << "\n";
                    printedSomething = true;
                }
                if (!printedSomething)
                {
                    // Optionally, you can print a message if the traced instruction isn’t in any stage.
                    cout << "Instruction #" << knob5_num << " is not involved in any current pipeline buffers.\n";
                }
            }
            pipeline[4] = pipeline[3];   // MEM → WB
            pipeline[3] = pipeline[2];   // EX → MEM
            pipeline[2] = pipeline[1];   // ID → EX
            pipeline[1] = pipeline[0];   // IF → ID
            pipeline[0].instr = nullptr; // Clear IF
        }

        if (!IAG_CALLED && (stall || branchPrediction))
        {
            IAG(curr_pc);
        }
        else if (!IAG_CALLED)
        {
            IAG(curr_pc, true);
        }

        cout << "NEW PC IS: 0x" << hex << global_pc << dec << endl;

        cout << "\n";
        if (!knob2)
            stall--;

        // cout<<"Memory"<<endl;
        // for(auto it = MainMemory.begin(); it!=MainMemory.end(); it++){
        //     cout<<hex<<it->first<<" "<<it->second<<endl;
        // }
        // cout<<"\n"<<endl;

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
        if (step)
        {
            // Wait for user input to proceed to the next cycle
            cout << "1: Continue Step, 0: step =false" << endl;

            cin >> step;

            if (step == 0)
            {
                continue;
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(time));
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
    cout << "=== REGISTERS ===" << endl;
    for (int i = 0; i < 31; i++)
        cout << "x" << i << " :" << RegFile[i] << endl;
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

    
    cout<<"\n=== MAIN MEMORY ==="<<endl;  
    for (auto it : MainMemory)
    {
        cout << "0x" << hex << it.first <<dec<< " :" << it.second << dec << endl;
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