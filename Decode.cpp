#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include <unordered_map>
#include <bits/stdc++.h>
using namespace std;

int global_pc = 0x0;
uint32_t IR = 0x0;
vector<pair<int, int>> InstructionPCPairs;
int32_t rz;

// Register names (x0 to x31)
const string regNums[32] = {
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "10", "11", "12", "13", "14", "15",
    "16", "17", "18", "19", "20", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "30", "31"};

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
    {"01100111100000001", "REM"}};

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
    {"1100111000", "JALR"}};

unordered_map<string, string> sTypeInstructions = {
    {"0100011000", "SB"}, // Store Byte
    {"0100011001", "SH"}, // Store Halfword
    {"0100011010", "SW"}, // Store Word
    {"0100011011", "SD"}  // Store Double-word (RV64I only)
};

unordered_map<string, string> sbTypeInstructions = {
    {"1100011000", "BEQ"}, // Branch if Equal
    {"1100011001", "BNE"}, // Branch if Not Equal
    {"1100011100", "BLT"}, // Branch if Less Than
    {"1100011101", "BGE"}  // Branch if Greater Than or Equal
};

unordered_map<string, string> uTypeInstructions = {
    {"0010111", "AUIPC"}, // Add Upper Immediate to PC
    {"0110111", "LUI"}    // Load Upper Immediate
};

unordered_map<string, string> ujTypeInstructions = {
    {"1101111", "JAL"} // Jump and Link
};

// Convention : Anything to do with PC , just call IAG with proper parameters , nothing else
int IAG(int ra, int imm)
{
    if (ra == 0 && imm == 0)
    {
        cout << "IAG Call; new PC: " << global_pc + 4 << endl;
        return global_pc = global_pc + 4;
    }
    else if (ra == 0 && imm != 0)
    {
        cout << "IAG Call; new PC: " << global_pc + imm << endl;
        return global_pc = global_pc + imm;
    }
    else
    {
        cout << "IAG Call; new PC: " << RegFile[ra] + imm << endl;
        return global_pc = RegFile[ra] + imm;
    }
}

// Convention : If action==write , then load , if action==read then store, if action==NULL then fetch instruction
int PMI(int EA, int pc, int data, int ra, string action = NULL)
{
    if (action == "write")
    {
        cout << "PMI Call; Writing " << data << " to memory address " << EA << endl;
        // Update Main Memory with data , to be filled later
        return 0;
    }
    else if (action == "read")
    {
        cout << "PMI Call; Reading from memory address " << EA << endl;
        // Read from Main Memory and fill RegFile , to be filled later
        return 0;
    }
    else
    {
        cout << "PMI Call; Fetching instruction from memory address " << pc << endl;
        IR = InstructionPCPairs[pc].second;
        return 0;
    }
}
uint32_t ALU(uint32_t val1, uint32_t val2, string OP)
{
    if (OP == "ADD" || OP == "ADDI" || OP == "LB" || OP == "LD" || OP == "LH" || OP == "LW" || OP == "JALR" || OP == "JAL" || OP == "SB" || OP == "SH" || OP == "SD" || OP == "SW")
    {
        return val1 + val2;
    }
    else if (OP == "BEQ")
        return val1 == val2;
    else if (OP == "BNE")
        return val1 != val2;
    else if (OP == "BGE")
        return val1 >= val2;
    else if (OP == "BLT")
        return val1 < val2;
    else if (OP == "AND" || OP == "ANDI")
        return rz = val1 & val2;
    else if (OP == "OR" || OP == "ORI")
        return rz = val1 | val2;
    else if (OP == "MUL")
        return rz = val1 * val2;
    else if (OP == "DIV")
        return rz = val1 / val2;
    else if (OP == "REM")
        return rz = val1 % val2;
    else if (OP == "XOR")
        return rz = val1 ^ val2;
    else if (OP == "SUB")
        return rz = val1 - val2;
    else if (OP == "SLL")
        return rz = val1 << val2;
    else if (OP == "SLT")
        return rz = val1 < val2;
    else if (OP == "SRL")
        return rz = val1 >> val2;
    else if (OP == "SRA")
    {
        rz = (int32_t)(val1 >> val2);
        return rz;
    }
}

// Function to decode R-type instructions
vector<string> decodeRType(uint32_t instruction)
{
    uint32_t opcode = instruction & 0x7F;         // bits [6:0]
    uint32_t rd = (instruction >> 7) & 0x1F;      // bits [11:7]
    uint32_t funct3 = (instruction >> 12) & 0x7;  // bits [14:12]
    uint32_t rs1 = (instruction >> 15) & 0x1F;    // bits [19:15]
    uint32_t rs2 = (instruction >> 20) & 0x1F;    // bits [24:20]
    uint32_t funct7 = (instruction >> 25) & 0x7F; // bits [31:25]

    string key = bitset<7>(opcode).to_string() +
                 bitset<3>(funct3).to_string() +
                 bitset<7>(funct7).to_string();

    if (rTypeInstructions.find(key) != rTypeInstructions.end())
    {
        return {rTypeInstructions[key], regNums[rd], regNums[rs1], regNums[rs2]};
    }
    return {"Unknown", to_string(instruction)};
}

vector<string> decodeIType(uint32_t instruction)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (int32_t)(instruction >> 20); // bits [31:20] (signed immediate)

    if (imm & 0x800)
        imm |= 0xFFFFF000; // Sign-extend 12-bit immediate

    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();

    if (iTypeInstructions.find(key) != iTypeInstructions.end())
    {
        return {iTypeInstructions[key], regNums[rd], regNums[rs1], to_string(imm)};
    }
    return {"Unknown"};
}

vector<string> decodeSType(uint32_t instruction)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm4_0 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm11_5 = (instruction >> 25) & 0x7F;

    int32_t imm = (imm11_5 << 5) | imm4_0;
    if (imm & (1 << 11))
        imm |= 0xFFFFF000; // Sign extension

    string key = bitset<7>(opcode).to_string() +
                 bitset<3>(funct3).to_string();

    if (sTypeInstructions.find(key) != sTypeInstructions.end())
    {
        return {sTypeInstructions[key], regNums[rs2], regNums[rs1], to_string(imm)};
    }
    return {"Unknown"};
}

vector<string> decodeSBType(uint32_t instruction)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm11 = (instruction >> 7) & 0x1;
    uint32_t imm4_1 = (instruction >> 8) & 0xF;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm10_5 = (instruction >> 25) & 0x3F;
    uint32_t imm12 = (instruction >> 31) & 0x1;

    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    if (imm & (1 << 12))
        imm |= 0xFFFFE000; // Sign extension

    string key = bitset<7>(opcode).to_string() + bitset<3>(funct3).to_string();

    if (sbTypeInstructions.find(key) != sbTypeInstructions.end())
    {
        return {sbTypeInstructions[key], regNums[rs1], regNums[rs2], to_string(imm)};
    }
    return {"Unknown"};
}

vector<string> decodeUType(uint32_t instruction)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t imm = (instruction >> 12);

    string key = bitset<7>(opcode).to_string();

    if (uTypeInstructions.find(key) != uTypeInstructions.end())
    {
        return {uTypeInstructions[key], regNums[rd], to_string(imm)};
    }
    return {"Unknown"};
}

vector<string> decodeUJType(uint32_t instruction)
{
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t imm19_12 = (instruction >> 12) & 0xFF;
    uint32_t imm11 = (instruction >> 20) & 0x1;
    uint32_t imm10_1 = (instruction >> 21) & 0x3FF;
    uint32_t imm20 = (instruction >> 31) & 0x1;

    int32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    if (imm & (1 << 20))
        imm |= 0xFFE00000; // Sign extension

    string key = bitset<7>(opcode).to_string();

    if (ujTypeInstructions.find(key) != ujTypeInstructions.end())
    {
        return {ujTypeInstructions[key], regNums[rd], to_string(imm)};
    }
    return {"Unknown"};
}

int Execute(string Type, string op, string rd, string rs1, string rs2, string imm)
{
    if (Type == "R")
    {
        cout << "Performing " << op << " operation on registers " << "x" << rs1 << " & " << "x" << rs2 << endl;
        uint32_t val = ALU(RegFile[stoi(rs1)], RegFile[stoi(rs2)], op);

        return val;
    }
    else if (Type == "I")
    {
        cout << "Performing " << op << " operation on register " << "x" << rs1 << " with immediate " << imm << endl;
        uint32_t val = ALU(RegFile[stoi(rs1)], stoi(imm), op);

        return val;
    }
    else if (Type == "SB")
    {
        return ALU(RegFile[stoi(rs1)], RegFile[stoi(rs2)], op);
    }
}

int main()
{
    ifstream inputFile("input.mc"); // Open input file
    if (!inputFile)
    {
        cerr << "Error: Unable to open input.mc" << endl;
        return 1;
    }

    vector<uint32_t> instructions;
    vector<pair<int, uint32_t>> InstructionPCPairs;

    string line;
    uint32_t machineCode;
    int pc;

    while (getline(inputFile, line))
    {
        istringstream iss(line);
        string address, machineCodeStr;

        // Read the address and machine code
        if (!(iss >> address >> machineCodeStr))
            continue;

        // Convert address (PC) from hex string to int
        if (address.back() == ':')
            address.pop_back(); // Remove trailing ':'

        pc = stoi(address, nullptr, 16);

        // Convert machine code from hex string to uint32_t
        machineCode = stoul(machineCodeStr, nullptr, 16);

        instructions.push_back(machineCode);
        InstructionPCPairs.emplace_back(pc, machineCode);
    }

    inputFile.close();
    vector<string> info;

    // Instruction execution loop
    while (global_pc / 4 < instructions.size())
    {
        // cout << "PC: " << hex << global_pc << "  |  ";
        IR = instructions[global_pc / 4]; // Fetch instruction

        uint32_t opcode = IR & 0x7F;

        if (opcode == 0x33)
        { // R-type
          // info = {op,rd,rs1,rs2}
            info = decodeRType(IR);
            Execute("R", info[0], info[1], info[2], info[3], "");

            global_pc += 4;
        }
        else if (opcode == 0x13 || opcode == 0x03)
        { // I-type
            // info =  {op,rd,rs1,imm}

            info = decodeIType(IR);
            Execute("I", info[0], info[1], info[2], "", info[3]);

            global_pc += 4;
        }
        else if (opcode == 0x23)
        { // S-type
            info = decodeSType(IR);
            global_pc += 4;
        }
        else if (opcode == 0x63)
        {                            // SB-type (Conditional Branch)
            int prev_pc = global_pc; // Store current PC before execution

            // info = {op,rs1,rs2,imm}

            info = decodeSBType(IR);

            // branch is either 1/0 and is calculated by ALU called via Execute.

            int branch = Execute("SB", info[0], "", info[1], info[2], info[3]);

            if (branch)
                global_pc += stoi(info[3]);
            else
                global_pc += 4;
        }
        else if (opcode == 0x17 || opcode == 0x37)
        { // U-type
            info = decodeUType(IR);
            global_pc += 4;
        }
        else if (opcode == 0x6F)
        { // UJ-type (JAL)
            info = decodeUJType(IR);

            int imm = stoi(info[2]);

            global_pc += imm; // Jump to new address
        }
        else
        {
            cout << "Unsupported instruction at PC: " << hex << global_pc << endl;
            global_pc += 4; // Skip unsupported instruction
        }
    }

    return 0;
}
