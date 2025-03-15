#include <iostream>
#include <fstream>
#include <bitset>
#include <unordered_map>

using namespace std;

// Register names (x0 to x31)
const string regNames[32] = {
    "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
    "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
    "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
    "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

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

    string key = bitset<7>(opcode).to_string() + " " + bitset<3>(funct3).to_string();

    if (iTypeInstructions.find(key) != iTypeInstructions.end()) {
        cout << iTypeInstructions[key] << " "
             << regNames[rd] << ", "
             << regNames[rs1]<<", "<<imm<< endl;
    } else {
        cout << "Unknown I-Type instruction" << endl;
    }
}

int main() {
    ifstream inputFile("input.mc");  // Open input file
    if (!inputFile) {
        cerr << "Error: Unable to open input.mc" << endl;
        return 1;
    }

    uint32_t machineCode;
    
    while (inputFile >> hex >> machineCode) {  // Read hexadecimal instructions
        uint32_t opcode = machineCode & 0x7F;

        if (opcode == 0x33) {  // R-type
            decodeRType(machineCode);
        } else if (opcode == 0x13 || opcode == 0x03) {  // I-type (ADDI, ORI, LW, etc.)
            decodeIType(machineCode);
        } else {
            cout << "Unsupported instruction format." << endl;
        }
    }

    inputFile.close();
    return 0;
}
