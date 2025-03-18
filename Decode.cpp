#include<bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include <unordered_map>
#include <bits/stdc++.h>
using namespace std;

int global_pc=0x0;

// Register names (x0 to x31)
const string regNames[32] = {
    "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
    "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
    "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
    "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

vector<int> RegFile(32, 0);

// 🔹 R-type instructions (opcode + funct3 + funct7 → instruction)
unordered_map<string, string> rTypeInstructions = {
    {"01100110000000000", "ADD"},
    {"01100110000100000", "SUB"},
    {"01100110010000000", "SLL"},
    {"01100110100000000", "SLT"},
    {"01100111010100000", "SRA"},
    {"01100111000000000", "XOR"},
    {"01100111010000000", "SRL"},
    {"01100111100000000", "OR"},
    {"01100111110000000", "AND"},
    {"01100110000000001", "MUL"},
    {"01100111000000001", "DIV"},
    {"01100111100000001", "REM"}
};

// 🔹 I-type instructions (opcode + funct3 → instruction)
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
    {"0100011000", "SB"},  // Store Byte
    {"0100011001", "SH"},  // Store Halfword
    {"0100011010", "SW"},  // Store Word
    {"0100011011", "SD"}   // Store Double-word (RV64I only)
};

unordered_map<string, string> sbTypeInstructions = {
    {"1100011000", "BEQ"},  // Branch if Equal
    {"1100011001", "BNE"},  // Branch if Not Equal
    {"1100011100", "BLT"},  // Branch if Less Than
    {"1100011101", "BGE"}   // Branch if Greater Than or Equal
};

unordered_map<string, string> uTypeInstructions = {
    {"0010111", "AUIPC"}, // Add Upper Immediate to PC
    {"0110111", "LUI"}    // Load Upper Immediate
};

unordered_map<string, string> ujTypeInstructions = {
    {"1101111", "JAL"}    // Jump and Link
};

//Convention : Anything to do with PC , just call IAG with proper parameters , nothing else
int IAG(int ra , int imm){
    if(ra==0 && imm==0){
        cout<<"IAG Call; new PC: "<<global_pc+4<<endl;
        return global_pc=global_pc+4;
    }
    else if(ra==0 && imm!=0){
        cout<<"IAG Call; new PC: "<<global_pc+imm<<endl;
        return global_pc=global_pc+imm;
    }
    else {
        cout<<"IAG Call; new PC: "<<RegFile[ra]+imm<<endl;
        return global_pc=RegFile[ra]+imm;
    }
}

int PMI 
// Function to decode R-type instructions
void decodeRType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;          // bits [6:0]
    uint32_t rd = (instruction >> 7) & 0x1F;       // bits [11:7]
    uint32_t funct3 = (instruction >> 12) & 0x7;   // bits [14:12]
    uint32_t rs1 = (instruction >> 15) & 0x1F;     // bits [19:15]
    uint32_t rs2 = (instruction >> 20) & 0x1F;     // bits [24:20]
    uint32_t funct7 = (instruction >> 25) & 0x7F;  // bits [31:25]

    string key = bitset<7>(opcode).to_string() +
                 bitset<3>(funct3).to_string() +
                 bitset<7>(funct7).to_string();

    if (rTypeInstructions.find(key) != rTypeInstructions.end()) {
        cout << rTypeInstructions[key] << " "
             << regNames[rd] << ", "
             << regNames[rs1] << ", "
             << regNames[rs2] << endl;
    } else {
        cout << "Unknown R-Type instruction "<<hex<<instruction<<" "<<key<<" " << endl;
        
       
    }
}

// Function to decode I-type instructions

//
void decodeIType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;          // bits [6:0]
    uint32_t rd = (instruction >> 7) & 0x1F;       // bits [11:7]
    uint32_t funct3 = (instruction >> 12) & 0x7;   // bits [14:12]
    uint32_t rs1 = (instruction >> 15) & 0x1F;     // bits [19:15]
    int32_t imm = (int32_t)(instruction >> 20);    // bits [31:20] (signed immediate)

    // Sign-extend the immediate (12-bit to 32-bit)
    if (imm & 0x800) imm |= 0xFFFFF000;

    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();

    if (iTypeInstructions.find(key) != iTypeInstructions.end()) {
        cout << iTypeInstructions[key] << " "
             << regNames[rd] << ", "
             << regNames[rs1]<<", "<<imm<< endl;
    } else {
        cout << "Unknown I-Type instruction" << endl;
    }
}

void decodeSType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm4_0 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm11_5 = (instruction >> 25) & 0x7F;
    
    // Compute full immediate value
    int32_t imm = (imm11_5 << 5) | imm4_0;
    if (imm & (1 << 11)) imm |= 0xFFFFF000; // Sign extension

    string key = bitset<7>(opcode).to_string() +
                 bitset<3>(funct3).to_string();

    if (sTypeInstructions.find(key) != sTypeInstructions.end()) {
        cout << sTypeInstructions[key] << " "
             << regNames[rs2] << ", " << imm << "(" << regNames[rs1] << ")"
             << endl;
    }
}

void decodeSBType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm11 = (instruction >> 7) & 0x1;
    uint32_t imm4_1 = (instruction >> 8) & 0xF;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm10_5 = (instruction >> 25) & 0x3F;
    uint32_t imm12 = (instruction >> 31) & 0x1;
    //cout<<imm11<<" "<<imm4_1<<" "<<imm10_5<<" "<<imm12<<endl;

    // Compute full immediate value (sign-extended)
    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    if (imm & (1 << 12)) imm |= 0xFFFFE000; // Sign extension
    //cout<<imm<<endl;

    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();

    if (sbTypeInstructions.find(key) != sbTypeInstructions.end()) {
        cout <<  sbTypeInstructions[key] << " "
             << regNames[rs1] << ", " << regNames[rs2] << ", " << imm
             << endl;
    }
}

void decodeUType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t imm = (instruction>>12); // Upper 20 bits

    string key = bitset<7>(opcode).to_string();
    
    if (uTypeInstructions.find(key) != uTypeInstructions.end()) {
        cout 
             << uTypeInstructions[key] << " "
             << regNames[rd] << ", " <<dec<< imm
             << endl;
    }
}

void decodeUJType(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t imm20 = (instruction >> 31) & 0x1;
    uint32_t imm10_1 = (instruction >> 21) & 0x3FF;
    uint32_t imm11 = (instruction >> 20) & 0x1;
    uint32_t imm19_12 = (instruction >> 12) & 0xFF;

    // Compute full immediate (sign-extended)
    int32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    if (imm & (1 << 20)) imm |= 0xFFE00000; // Sign extend

    string key = bitset<7>(opcode).to_string();

    if (ujTypeInstructions.find(key) != ujTypeInstructions.end()) {
        cout 
             << ujTypeInstructions[key] << " "
             << regNames[rd] << ", "<<dec << imm
             << endl;
    }
}

int main() {
    ifstream inputFile("input.mc");  // Open input file
    if (!inputFile) {
        cerr << "Error: Unable to open input.mc" << endl;
        return 1;
    }

    vector<uint32_t> instructions;
    uint32_t machineCode;
    global_pc = 0x0;  // Start PC at 0x0

    // Read instructions from file into memory (vector)
    while (inputFile >> hex >> machineCode) {
        instructions.push_back(machineCode);
    }

    inputFile.close();

    unordered_map<int, int> registers;  // Simulate registers x0 - x31
    registers[0] = 0;  // x0 is always 0

    // Instruction execution loop
    while (global_pc / 4 < instructions.size()) {
        cout << "PC: " << hex << global_pc << "  |  ";
        uint32_t instruction = instructions[global_pc / 4];  // Fetch instruction

        uint32_t opcode = instruction & 0x7F;

        if (opcode == 0x33) {  // R-type
            decodeRType(instruction);
            global_pc += 4;
        } 
        else if (opcode == 0x13 || opcode == 0x03) {  // I-type
            decodeIType(instruction);
            global_pc += 4;
        } 
        else if (opcode == 0x23) {  // S-type
            decodeSType(instruction);
            global_pc += 4;
        } 
        else if (opcode == 0x63) {  // SB-type (Conditional Branch)
            int prev_pc = global_pc;  // Store current PC before execution
            decodeSBType(instruction);

            // Extract rs1, rs2, funct3 for branch conditions
            uint32_t funct3 = (instruction >> 12) & 0x7;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            uint32_t rs2 = (instruction >> 20) & 0x1F;

            int32_t imm = ((instruction >> 31) & 0x1) << 12 | 
                          ((instruction >> 7) & 0x1) << 11 | 
                          ((instruction >> 25) & 0x3F) << 5 | 
                          ((instruction >> 8) & 0xF) << 1;

            if (imm & 0x1000) imm |= 0xFFFFE000; // Sign extend

            // Apply branch conditions
            if ((funct3 == 0x0 && registers[rs1] == registers[rs2]) ||  // BEQ
                (funct3 == 0x1 && registers[rs1] != registers[rs2]) ||  // BNE
                (funct3 == 0x4 && registers[rs1] < registers[rs2]) ||   // BLT
                (funct3 == 0x5 && registers[rs1] >= registers[rs2])) {  // BGE
                global_pc += imm;  // Take branch
            } else {
                global_pc += 4;  // Skip branch
            }
        } 
        else if (opcode == 0x17 || opcode == 0x37) {  // U-type
            decodeUType(instruction);
            global_pc += 4;
        } 
        else if (opcode == 0x6F) {  // UJ-type (JAL)
            decodeUJType(instruction);

            int32_t imm = ((instruction >> 31) & 0x1) << 20 |
                          ((instruction >> 12) & 0xFF) << 12 |
                          ((instruction >> 20) & 0x1) << 11 |
                          ((instruction >> 21) & 0x3FF) << 1;

            if (imm & 0x100000) imm |= 0xFFE00000; // Sign extend

            global_pc += imm;  // Jump to new address
        } 
        else {
            cout << "Unsupported instruction at PC: " << hex << global_pc << endl;
            global_pc += 4;  // Skip unsupported instruction
        }
    }

    return 0;
}

