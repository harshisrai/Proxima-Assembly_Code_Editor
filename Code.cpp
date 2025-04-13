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

int RM = 0x0;
int RZ = 0x0;
int RY = 0x0;

struct Instruction {
    uint32_t mc;
    string needs_rs1_in;
    string needs_rs2_in;
    string op;
    int rd;
    int rs1;
    int rs2;
    int32_t imm;
   
};

struct PipelineStage {
    Instruction* instr = nullptr;
};

bool needsForwarding(const Instruction& curr, const Instruction& prev,string in) {
    if (prev.op == "NOP" || prev.rd == -1) return false;
    
    return ((curr.rs1 == prev.rd && curr.needs_rs1_in==in) || (curr.rs2 == prev.rd && curr.needs_rs2_in==in));
}

bool loadUseHazard(const Instruction& curr, const Instruction& prev) {
    
    return (prev.op == "LW" ||prev.op=="LH" && prev.op=="LB") &&
           ((curr.rs1 == prev.rd && curr.needs_rs1_in=="EX") || (curr.rs2 == prev.rd && curr.needs_rs2_in=="EX"));
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
    {"01100111100000001", "REM"}
};

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
    {"1100111000", "JALR"}
};

unordered_map<string, string> sTypeInstructions = {
    {"0100011000", "SB"},
    {"0100011001", "SH"},
    {"0100011010", "SW"},
    {"0100011011", "SD"}
};

unordered_map<string, string> sbTypeInstructions = {
    {"1100011000", "BEQ"},
    {"1100011001", "BNE"},
    {"1100011100", "BLT"},
    {"1100011101", "BGE"}
};

unordered_map<string, string> uTypeInstructions = {
    {"0010111", "AUIPC"},
    {"0110111", "LUI"}
};

unordered_map<string, string> ujTypeInstructions = {
    {"1101111", "JAL"}
};

vector<int> RegFile={0,0,2147483612,268435456,0,0,0,0,0,0,1,2147483612,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

const int regNums[32] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

Instruction decodeRType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string() + bitset<7>(funct7).to_string();
    if (rTypeInstructions.find(key) != rTypeInstructions.end()) return {instruction,"EX","EX",rTypeInstructions[key], regNums[rd], regNums[rs1], regNums[rs2]};
    return {instruction,"Unknown"};
}

Instruction decodeIType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (int32_t)(instruction >> 20);
    if (imm & 0x800) imm |= 0xFFFFF000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    if (iTypeInstructions.find(key) != iTypeInstructions.end()) return {instruction,"EX","",iTypeInstructions[key], regNums[rd], regNums[rs1], imm};
    return {instruction,"Unknown"};
}

Instruction decodeSType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm4_0 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm11_5 = (instruction >> 25) & 0x7F;
    RM = RegFile[rs2];
    int32_t imm = (imm11_5 << 5) | imm4_0;
    if (imm & (1 << 11)) imm |= 0xFFFFF000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
   
    if (sTypeInstructions.find(key) != sTypeInstructions.end()) return {instruction,"MEM","EX",sTypeInstructions[key],-1, regNums[rs2], regNums[rs1], imm};
    return {instruction,"Unknown"};
}

Instruction decodeSBType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm11 = (instruction >> 7) & 0x1;
    uint32_t imm4_1 = (instruction >> 8) & 0xF;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    int rs1 = (instruction >> 15) & 0x1F;
    int rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm10_5 = (instruction >> 25) & 0x3F;
    uint32_t imm12 = (instruction >> 31) & 0x1;
    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    if (imm & (1 << 12)) imm |= 0xFFFFE000;
    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();
    if (sbTypeInstructions.find(key) != sbTypeInstructions.end()) return {instruction,"EX","EX",sbTypeInstructions[key], regNums[rs1], regNums[rs2], imm};
    return {instruction,"Unknown"};
}

Instruction decodeUType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    int32_t imm = (instruction >> 12);
    string key = bitset<7>(opcode).to_string();
    if (uTypeInstructions.find(key) != uTypeInstructions.end()) return {instruction,"","",uTypeInstructions[key], regNums[rd], imm};
    return {instruction,"Unknown"};
}

Instruction decodeUJType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    int rd = (instruction >> 7) & 0x1F;
    uint32_t imm19_12 = (instruction >> 12) & 0xFF;
    uint32_t imm11 = (instruction >> 20) & 0x1;
    uint32_t imm10_1 = (instruction >> 21) & 0x3FF;
    uint32_t imm20 = (instruction >> 31) & 0x1;
    int32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    if (imm & (1 << 20)) imm |= 0xFFE00000;
    string key = bitset<7>(opcode).to_string();
    if (ujTypeInstructions.find(key) != ujTypeInstructions.end()) return {instruction,"","",ujTypeInstructions[key], regNums[rd], imm};
    return {instruction,"Unknown"};
}

Instruction decodeInstruction(uint32_t instr) {
    uint32_t opcode = instr & 0x7F;
    switch (opcode) {
        case 0x33: return decodeRType(instr);
        case 0x13:
        case 0x03:
        case 0x67: return decodeIType(instr);
        case 0x23: return decodeSType(instr);
        case 0x63: return decodeSBType(instr);
        case 0x37:
        case 0x17: return decodeUType(instr);
        case 0x6F: return decodeUJType(instr);
        default: return {instr,"Unknown"};
    }
}

vector<Instruction> loadProgram(const string& filename) {
    vector<Instruction> program;
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        if (line.empty()) continue;
        uint32_t instr = stoul(line, nullptr, 16);
        program.push_back(decodeInstruction(instr));
    }
    return program;
}

int main() {
    vector<Instruction> program = loadProgram("instructions.txt");

    vector<PipelineStage> pipeline(5); // IF, ID, EX, MEM, WB
    int cycle = 0, pc = 0;
    Instruction nop;

    cout << "Pipeline simulation with forwarding and correct timing:\n\n";

    while (pc < program.size() || any_of(pipeline.begin(), pipeline.end(), [](auto& st){ return st.instr != nullptr; })) {
        cout << "Cycle " << ++cycle << ":\n";

        // Print WB stage
        if (pipeline[4].instr) cout << "  WriteBack:  " <<"0x"<<setfill('0')<<setw(8)<<hex<<pipeline[4].instr->mc << "\n";

        // Print MEM stage
        if (pipeline[3].instr) cout << "  MemAccess: " <<"0x"<<setfill('0')<<setw(8)<<hex<<pipeline[3].instr->mc << "\n";

        // Print EX stage
        if (pipeline[2].instr) {cout << "  Execute:  " <<"0x"<<setfill('0')<<setw(8)<<hex<<pipeline[2].instr->mc<<endl;
            Instruction& curr = *pipeline[2].instr;
           
            if(pipeline[3].instr && needsForwarding(curr,*pipeline[3].instr,"MEM")){
                cout<<"FORWARD From MEM/WB "<<endl;
            }
        
        
        
        }

        // Check hazard/stall/forwarding in ID before shifting pipeline
        bool stall = false;
        if (pipeline[1].instr) {
            Instruction& curr = *pipeline[1].instr;
            cout << "  Decode:  " <<"0x"<<setfill('0')<<setw(8)<<hex<<curr.mc;

            // Check load-use hazard with EX stage (before shifting)
            if (pipeline[2].instr && loadUseHazard(curr, *pipeline[2].instr)) {
                cout << "  (STALL due to load-use hazard) ";
                stall = true;
            } 
            else {if (pipeline[2].instr && needsForwarding(curr, *pipeline[2].instr,"EX")) {
                cout << "  (FORWARD from EX/MEM) ";
            }
             if (pipeline[3].instr && needsForwarding(curr, *pipeline[3].instr,"EX") ) {
                cout << "  (FORWARD from MEM/WB) ";
            }}
            // else if (pipeline[4].instr && needsForwarding(curr, *pipeline[4].instr)) {
            //     cout << "  (FORWARD from WB)";
            // }
            cout << "\n";
        }

        if (pc < program.size()) {
            cout << "  Fetch:  " <<"0x"<<setfill('0')<<setw(8)<<hex<< program[pc].mc << "\n";
        }

        // Shift pipeline stages (AFTER checking hazards)
        pipeline[4] = pipeline[3];
        pipeline[3] = pipeline[2];

        if (!stall) {
            pipeline[2] = pipeline[1];  // ID → EX
            if (pc < program.size()) {
                pipeline[1].instr = &program[pc++]; // IF → ID
            } else {
                pipeline[1].instr = nullptr;
            }
        } else {
            pipeline[2].instr = &nop; // Inject bubble
        }

        pipeline[0].instr = nullptr; // IF not used here

        cout << "\n";
    }

    return 0;
}

