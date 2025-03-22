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

// defining global structures that will be used throughout the process
int global_pc = 0x0;
uint32_t IR = 0x0;
uint32_t MDR = 0x0;
uint32_t MAR = 0x0;
uint32_t RM = 0x0;
uint32_t RZ = 0x0;
uint32_t RY = 0x0;
// Register file (x0 to x31) - initialized with preloaded values
vector<int> RegFile={0,0,2147483612,268435456,0,0,0,0,0,0,1,2147483612,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
vector<pair<int, uint32_t>> InstructionPCPairs;
unordered_map<uint32_t, uint8_t> MainMemory;//uint8_t cause byte

// Register names (x0 to x31)
const string regNums[32] = {
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "10", "11", "12", "13", "14", "15",
    "16", "17", "18", "19", "20", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "30", "31"};

// 🔹Set of R-type instructions (opcode + funct3 + funct7 → instruction)
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

// 🔹Set of I-type instructions (opcode + funct3 → instruction)
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

// Step 5 of the process , reduced to a function , reg x0 will always be 0
void WriteBack(int val, int rd)
{
    if (rd != 0)
    {
        cout << "Writing value 0x" <<hex<< val << " to register x" << dec << rd << endl;
        RegFile[rd] = val;
    }
}

void MemAccess(string op, int value, int eff)
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
            st.insert(eff+i-(eff+i)%4);
        }
        cout<<"STARTMEMORY"<<endl;
        for(auto it:st){
            cout<<"0x"<<it<<": ";
            for(int i =0;i<4;i++){
                cout << std::setw(2) << std::setfill('0') 
                << static_cast<int>(MainMemory[it + i]) << " ";
            }
            cout<<endl;
        }
        cout<<"ENDMEMORY"<<endl;
    }
    else if (op == "LB" || op == "LH" || op == "LW")
    {
        int size = 0; // Number of bytes to load
        int32_t value = 0;

        if (op == "LB")
            size = 1;
        else if (op == "LH")
            size = 2;
        else if (op == "LW")
            size = 4;

        // Reconstruct the value in little-endian order
        for (int i = size - 1; i >= 0; i--)
        {
            value = (value << 8) | MainMemory[eff + i];
        }

        // Sign-extension for lb and lh
        if (op == "LB" && (value & 0x80))
        {
            value |= 0xFFFFFF00; // Extend sign for 8-bit value
        }
        else if (op == "LH" && (value & 0x8000))
        {
            value |= 0xFFFF0000; // Extend sign for 16-bit value
        }
        cout << "MDR is updated with value " << value << " from address 0x" << hex << eff << endl;
        MDR = value;
    }
}

// PC generator , the if /else-if cases illustrate the functioning of the MUXes in the flow
//  Convention : Anything to do with PC , just call IAG with proper parameters , nothing else
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

// PMI does (a)perform store , (b)perform load , (c)fetch the instruction of pc calculated from IAG
//(a) is done by writiing val in MDR to address in MAR
//(b) is done by writing val in MDR to register ra
//(c) is done by updating IR
//  Convention : If action==write , then load , if action==read then store, if action==NULL then fetch instruction
void PMI(int EA, int pc, int data, int ra, string action = "")
{
    if (action == "LB" || action == "LH" || action == "LW")
    {
        cout << "PMI Call; Writing 0x" <<hex<< data <<dec<< " from MDR to RY "<< endl;
        MemAccess(action, data, EA);
    }
    else if (action == "SB" || action == "SH" || action == "SW")
    {
        cout << "PMI Call; Writing 0x" <<hex<< data <<dec<< " from MDR to memory address 0x" <<hex<< EA <<dec<< " which was stored in MAR" << endl;
        MemAccess(action, data, EA);
    }
    else
    {
        cout << "PMI Call; Fetching instruction from PC 0x" <<hex<< pc <<dec<< " and loading into IR" << endl;
        IR = InstructionPCPairs[(pc / 4)].second;
    }
}

// depending on operation
uint32_t ALU(uint32_t val1, uint32_t val2, string OP)
{
    if (OP == "ADD" || OP == "ADDI" || OP == "LB" || OP == "LD" || OP == "LH" || OP == "LW" || OP == "JALR" || OP == "JAL" || OP == "SB" || OP == "SH" || OP == "SD" || OP == "SW")
    {
        return val1 + val2;
    }
    else if (OP == "BEQ"){
        
        return val1 == val2;}
    else if (OP == "BNE")
        return val1 != val2;
    else if (OP == "BGE")
        return val1 >= val2;
    else if (OP == "BLT")
        return val1 < val2;
    else if (OP == "AND" || OP == "ANDI")
        return RZ = val1 & val2;
    else if (OP == "OR" || OP == "ORI")
        return RZ = val1 | val2;
    else if (OP == "MUL")
        return RZ = val1 * val2;
    else if (OP == "DIV")
        if(val2==0)
        {
            throw runtime_error("Division by zero error");
            return 0;
        }
        else
        return RZ = val1 / val2;
    else if (OP == "REM")
        return RZ = val1 % val2;
    else if (OP == "XOR")
        return RZ = val1 ^ val2;
    else if (OP == "SUB")
        return RZ = val1 - val2;
    else if (OP == "SLL")
        return RZ = val1 << val2;
    else if (OP == "SLT")
        return RZ = val1 < val2;
    else if (OP == "SRL")
        return RZ = val1 >> val2;
    else if (OP == "SRA")
    {
        RZ = (int32_t)(val1 >> val2);
        return RZ;
    }
    else if (OP == "LUI")
    {
        return val1 << 12;
    }
    else if (OP == "AUIPC")
    {
        return (val1 << 12) + val2;
    }
    else
    {
        cout << "Invalid Operation in ALU" << endl;
        return 0;
    }
}

// Decoding breakds down the fields of the instructions and reads source regs
vector<string> decodeRType(uint32_t instruction)
{
    cout << "Decoding instruction " << "0x" << hex << instruction << endl;
    uint32_t opcode = instruction & 0x7F;         // bits [6:0]
    uint32_t rd = (instruction >> 7) & 0x1F;      // bits [11:7]
    uint32_t funct3 = (instruction >> 12) & 0x7;  // bits [14:12]
    uint32_t rs1 = (instruction >> 15) & 0x1F;    // bits [19:15]
    uint32_t rs2 = (instruction >> 20) & 0x1F;    // bits [24:20]
    uint32_t funct7 = (instruction >> 25) & 0x7F; // bits [31:25]
    cout << "Value " << RegFile[rs1] << " has been read from register x" << dec << rs1 << endl;
    cout << "Value " << RegFile[rs2] << " has been read from register x" << dec << rs2 << endl;
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
    cout << "Decoding instruction " << "0x" << hex << instruction << endl;
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (int32_t)(instruction >> 20); // bits [31:20] (signed immediate)
    cout << "Value " << RegFile[rs1] << " has been read from register x" << dec << rs1 << endl;
    cout << "Immediate value 0x" << hex << imm << " has been read" << endl;
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
    cout << "Decoding instruction " << "0x" << hex << instruction << endl;
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm4_0 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm11_5 = (instruction >> 25) & 0x7F;
    cout << "Value " << RegFile[rs1] << " has been read from register x" << dec << rs1 << endl;
    cout << "Value " << RegFile[rs2] << " has been read from register x" << dec << rs2 << endl;
    // For store instructions (S-type: SW, SB, etc.):
    RM = RegFile[rs2]; // info[1] = rs2 in your decodeS output
    cout << "RM has been fed with value from register x" << dec << rs2 << endl;
    int32_t imm = (imm11_5 << 5) | imm4_0;
    cout << "Immediate value 0x" << hex << imm << " has been read" << endl;
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
    cout << "Decoding instruction " << "0x" << hex << instruction << endl;
    uint32_t opcode = instruction & 0x7F;
    uint32_t imm11 = (instruction >> 7) & 0x1;
    uint32_t imm4_1 = (instruction >> 8) & 0xF;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm10_5 = (instruction >> 25) & 0x3F;
    uint32_t imm12 = (instruction >> 31) & 0x1;

    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    cout << "Value " << RegFile[rs1] << " has been read from register x" << dec << rs1 << endl;
    cout << "Value " << RegFile[rs2] << " has been read from register x" << dec << rs2 << endl;
    cout << "Immediate value 0x" << hex << imm << " has been read" << endl;
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
    cout << "Decoding instruction " << "0x" << hex << instruction << endl;
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t imm = (instruction >> 12);
    cout << "Immediate value 0x" << hex << imm << " has been read" << endl;
    string key = bitset<7>(opcode).to_string();

    if (uTypeInstructions.find(key) != uTypeInstructions.end())
    {
        return {uTypeInstructions[key], regNums[rd], to_string(imm)};
    }
    return {"Unknown"};
}

vector<string> decodeUJType(uint32_t instruction)
{
    cout << "Decoding instruction " << "0x" << hex << instruction << endl;
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t imm19_12 = (instruction >> 12) & 0xFF;
    uint32_t imm11 = (instruction >> 20) & 0x1;
    uint32_t imm10_1 = (instruction >> 21) & 0x3FF;
    uint32_t imm20 = (instruction >> 31) & 0x1;

    int32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
    cout << "Immediate value 0x" << hex << imm << " has been read" << endl;
    if (imm & (1 << 20))
        imm |= 0xFFE00000; // Sign extension

    string key = bitset<7>(opcode).to_string();

    if (ujTypeInstructions.find(key) != ujTypeInstructions.end())
    {
        return {ujTypeInstructions[key], regNums[rd], to_string(imm)};
    }
    return {"Unknown"};
}

// performing execution stage , respective to the op , updating RZ too
int Execute(string Type, string op, string rd, string rs1, string rs2, string imm)
{
    if (Type == "R")
    {
        cout << " ALU Performing " << op << " operation on registers " << "x" << rs1 << " & " << "x" << rs2 << endl;
        uint32_t val = ALU(RegFile[stoi(rs1)], RegFile[stoi(rs2)], op);
        RZ = val;
        cout << "ALU output has been fed to RZ" << endl;
        return val;
    }
    else if (Type == "I")
    { // if op is not lb lh lw or ld , then do the below cout
        if (op != "LB" && op != "LH" && op != "LW" && op != "LD")
        {
            cout << " ALU Performing " << op << " operation on register " << "x" << rs1 << " with immediate " << imm << endl;
        }
        else
        {
            cout << "ALU calculating effective address for " << op << " operation using register x" << rs1 << " and immediate " << imm << endl;
        }
        uint32_t val = ALU(RegFile[stoi(rs1)], stoi(imm), op);
        RZ = val;
        cout << "ALU output has been fed to RZ" << endl;

        return val;
    }
    else if (Type == "SB")
    {
        cout << " ALU Performing " << op << " operation on registers " << "x" << rs1 << " & " << "x" << rs2 << endl;
        return ALU(RegFile[stoi(rs1)], RegFile[stoi(rs2)], op);
    }

    else if (Type == "S")
    {
        cout << "ALU calculating effective address using register x" << rs1 << " and immediate " << imm << endl;
        return ALU(RegFile[stoi(rs1)], stoi(imm), op);
    }
    else if (Type == "U")
    {
        cout << "ALU performing " << op << " on immediate " << imm << endl;
        uint32_t val = ALU(stoi(imm), global_pc, op);
        RZ = val;
        cout << "ALU output has been fed to RZ" << endl;
        return val;
    }
    else
    {
        cout << "Invalid Instruction Type in Execute Stage" << endl;
        return 0;
    }
}

std::string to_uppercase(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

void dumpMemoryToMCFile(const std::unordered_map<uint32_t, uint8_t>& memory,const std::string& filename)
{
    // If memory is empty, just create an empty file.
    if (memory.empty()) {
        std::ofstream emptyFile(filename);
        return;
    }

    // Determine the unique 4-byte aligned addresses (groups) that exist in the map.
    std::unordered_set<uint32_t> groupAddresses;
    for (const auto& kv : memory) {
        uint32_t groupAddr = kv.first & ~0x3; // Align address to a 4-byte boundary.
        groupAddresses.insert(groupAddr);
    }

    // Copy to a vector and sort in descending order.
    std::vector<uint32_t> groups(groupAddresses.begin(), groupAddresses.end());
    std::sort(groups.begin(), groups.end(), std::greater<uint32_t>());

    // Open the output file.
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: could not open output file " << filename << std::endl;
        return;
    }

    // Print a header.
    outFile << "  Address   +3  +2  +1  +0" << "\n";

    // Iterate through each group address.
    for (uint32_t addr : groups) {
        // Print the 4-byte aligned address.
        outFile << "0x" 
                << std::uppercase 
                << std::hex 
                << std::setw(8) 
                << std::setfill('0') 
                << addr 
                << std::dec 
                << std::nouppercase 
                << std::setfill(' ');

        // For each word, print the 4 bytes in order: +3, +2, +1, +0.
        for (int offset = 3; offset >= 0; --offset) {
            uint32_t currentAddress = addr + offset;
            auto it = memory.find(currentAddress);
            uint8_t value = (it != memory.end()) ? it->second : 0; // Default to 0 if not found

            outFile << "  " 
                    << std::setw(2)
                    << std::uppercase
                    << std::hex
                    << std::setfill('0')
                    << static_cast<int>(value)
                    << std::dec
                    << std::nouppercase
                    << std::setfill(' ');
        }
        outFile << "\n";
    }
    cout<<"Memory Dumped to memory.mc"<<endl;
    outFile.close();
}

void dumpRegisterToMcFile(){
    std::ofstream outFile("Registers.mc");
    
    if (!outFile) { // Check if the file opened successfully
        std::cerr << "Error: Could not open file for writing.\n";
        return ;
    }

    // Write register values to the file
    for (size_t i = 0; i < RegFile.size(); i++) {
        outFile << "x" << i << ": "  << RegFile[i] << "\n";  // Writing in decimal format
    }

    // Close the file
    outFile.close();

    std::cout << "Register values written to Registers.mc\n";
}

int main()
{
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
            uint32_t memVal = stoul(memValStr, nullptr, 16);

            // Update MainMemory with the value at this address.
            MemAccess("SW", memVal, memAddr);
        }
    }
    inputFile.close();
    int clock = 0;

    while (global_pc / 4 < InstructionPCPairs.size())
    {
        cout << "============================" << endl;
        cout << "FDEMW Cycle Start: PC = 0x" << hex << global_pc << dec << endl;
        int current_pc = global_pc;

        // ----- FETCH STAGE -----
        cout << "\n----- FETCH STAGE -----" << endl;
        // sleep for 100 milliseconds
        //this_thread::sleep_for(chrono::milliseconds(100));
        PMI(0, current_pc, 0, 0); // This call fetches the instruction into IR.
        cout << "[MAIN] Fetched Instruction: 0x" << hex << IR << dec << endl;

        // ----- DECODE STAGE -----
        cout << "\n----- DECODE STAGE -----" << endl;
        // sleep for 100 milliseconds
        //this_thread::sleep_for(chrono::milliseconds(100));
        uint32_t opcode = IR & 0x7F;
        vector<string> info;
        int alu_output = 0;
        // The decode functions themselves print the decoded information.
        if (opcode == 0x33) // R-type
            info = decodeRType(IR);
        else if (opcode == 0x13 || opcode == 0x03 || opcode == 0x67) // I-type
            info = decodeIType(IR);
        else if (opcode == 0x23) // S-type
            info = decodeSType(IR);
        else if (opcode == 0x63) // SB-type
            info = decodeSBType(IR);
        else if (opcode == 0x17 || opcode == 0x37) // U-type
            info = decodeUType(IR);
        else if (opcode == 0x6F) // UJ-type
            info = decodeUJType(IR);
        else
            cout << "[DECODE] Unsupported instruction at PC: 0x" << hex << global_pc << dec << endl;

        // ----- EXECUTE STAGE -----
        cout << "\n----- EXECUTE STAGE -----" << endl;
        // sleep for 100 milliseconds
        //this_thread::sleep_for(chrono::milliseconds(100));
        if (opcode == 0x33)
        { // R-type instruction (e.g., ADD, SUB, etc.)
            alu_output = Execute("R", info[0], info[1], info[2], info[3], "");
            RZ = alu_output;
        }
        else if (opcode == 0x13 || opcode == 0x03 || opcode == 0x67)
        { // I-type instruction (e.g., ADDI, LW, JALR, etc.)
            alu_output = Execute("I", info[0], info[1], info[2], "", info[3]);
            RZ = alu_output;
        }
        else if (opcode == 0x23)
        { // S-type instruction (store operations)
            // For store, compute effective address.
            alu_output = ALU(RegFile[stoi(info[2])], stoi(info[3]), info[0]);
            RZ = alu_output;
            cout << "[EXECUTE] Effective Address: " << alu_output << endl;
        }
        else if (opcode == 0x63)
        { // SB-type instruction (conditional branch)
            alu_output = Execute("SB", info[0], "", info[1], info[2], info[3]);
            RZ = alu_output;
        }
        else if (opcode == 0x17 || opcode == 0x37)
        { // U-type instruction (AUIPC, LUI)
            alu_output = Execute("U", info[0], info[1], "", "", info[2]);
            RZ = alu_output;
        }
        else if (opcode == 0x6F)
        {                   // UJ-type instruction (JAL)
            alu_output = 0; // For JAL, write-back is handled separately.
            RZ = alu_output;
        }
        else
        {
            cout << "[EXECUTE] No execution performed for unsupported opcode." << endl;
        }

        // ----- MEMORY STAGE -----
        cout << "\n----- MEMORY STAGE -----" << endl;
        // sleep for 100 milliseconds
        //this_thread::sleep_for(chrono::milliseconds(100));
        MAR = RZ;
        cout << "MAR has been fed with effective address from RZ" <<RZ<< endl;
        if (opcode == 0x23)
        { // S-type: store operation has already computed the effective address.
            MDR = RM;
            PMI(MAR, current_pc, RM, 0,to_uppercase(info[0]));
            // (Assuming PMI would update the memory in a real implementation)
            cout << "Value " << RM << " from RM has been written to memory address 0x" << hex << alu_output << endl;
        }
        else if (opcode == 0x03) // Example for load operations (if implemented)
        {
            // CHANGE WITH RESPECT TO HARDIK'S CODE
            MDR = MainMemory[MAR];
            cout << "MDR has been fed with value" << MainMemory[MAR] << " from memory address 0x" << hex << MAR << " which was stored in MAR" << endl;
            PMI(MAR, current_pc, 0, stoi(info[1]),to_uppercase(info[0]));

            // (Assuming PMI would update the register in a real implementation)
        }

        // ----- WRITE-BACK STAGE -----
        cout << "\n----- WRITE-BACK STAGE -----" << endl;
        // sleep for 100 milliseconds
       // this_thread::sleep_for(chrono::milliseconds(100));
        if (opcode == 0x33 || opcode == 0x13)
        {
            RY = RZ;
            cout << "As it is R type instruction, RY is updated with data from RZ" << endl;
            WriteBack(RY, stoi(info[1]));
        }
        // if load , RY = MDR
        else if (opcode == 0x03)
        {
            RY = MDR;
            cout << "As it is load instruction, RY is updated with data from MDR" << endl;
            WriteBack(RY, stoi(info[1]));
        }
        else if (opcode == 0x17 || opcode == 0x37)
        { // U-type
            RY = RZ;
            cout << "As it is U-type (LUI/AUIPC), RY is updated with data from RZ" << endl;
            WriteBack(RY, stoi(info[1]));
        }

        // if jal or jalr , RY=PC+4
        else if (opcode == 0x6F || opcode == 0x67)
        {
            RY = global_pc + 4;
            cout << "As it is jal/jalr instruction, RY is updated to be PC+4" << endl;
            WriteBack(RY, stoi(info[1]));
        }
        // For branches and other control instructions, write-back may not be needed.)
        
        // ----- PC UPDATE -----
        cout << "\n----- PC UPDATE -----" << endl;
        // sleep for 100 milliseconds
       // this_thread::sleep_for(chrono::milliseconds(100));
        if (opcode == 0x67)
        { // JALR
            global_pc = alu_output;
            cout << "[PC UPDATE] JALR - New PC: " << global_pc << endl;
        }
        else if (opcode == 0x63)
        { // Branches
            cout<<alu_output<<endl;
            if (alu_output)
            {
                global_pc = IAG(0, stoi(info[3]));
            }
            else
            {
                global_pc = IAG(0, 4);
            }
        }
        else if (opcode != 0x6F) // For all instructions except JAL (handled below)
        {
            global_pc = IAG(0, 4);
        }
        else if (opcode == 0x6F)
        { // UJ-type (JAL)
            global_pc = IAG(0, stoi(info[2]));
        }

        cout << "\n===== Cycle End, New PC: 0x" << hex << global_pc << dec << " =====" << endl
             << endl;
        clock++;
        cout << "Clock Cycles: " << clock << endl;
        cout << "============================" << endl;
    }
    // print memory
    // cout << "Memory Contents:" << endl;
    // for (auto &pair : MainMemory)
    // {
    //     cout << "0x" << hex << pair.first << ": 0x" << hex << pair.second << dec << endl;
    // }
    //clear memory.mc before using
    ofstream memoryFile("memory.mc", ios::trunc);
    if (!memoryFile) {
        cerr << "Error: Unable to clear memory.mc" << endl;
        return 1;
    }
    memoryFile.close();
    
    dumpMemoryToMCFile(MainMemory, "memory.mc");
   dumpRegisterToMcFile();
    

    return 0;
}
