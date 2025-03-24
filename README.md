
# CS204 Project

This repository is a complete project developed for CS204. It consists of two main parts:

1. **RISC-V Simulator**  
   A C++ based simulator that decodes, executes, and simulates RISC-V machine instructions. It reads an input machine code file (generated from an assembly source) and performs a multi-stage simulation (Fetch, Decode, Execute, Memory, and Write-back). The simulation results, including memory and register dumps, are saved to files.

2. **Web Interface**  
   A Flask-based web application that serves as a user interface for the simulator. Users can submit assembly code, which is then assembled (via an external executable), simulated, and displayed in a user-friendly format. The interface shows the machine code, corresponding assembly instructions, and memory state.

## Project Structure

- **README.md**  
  This file describes the project, its structure, and how to use it.

- **phase2.cpp**  
  Contains the main simulator code. It implements:
  - Instruction decoding (for R, I, S, SB, U, and UJ types)
  - ALU operations
  - Memory access routines (for load and store instructions)
  - Control logic for updating the Program Counter (PC)
  - Output functions to dump memory (`memory.mc`) and registers (`Registers.mc`)

- **phase1.cpp**  
  (Not shown fully here)  
  Processes the assembly input file (`input.asm`), performs label resolution, and generates the machine code output (`input.mc`).

- **app.py**  
  A Flask application that:
  - Accepts assembly code input via a web form.
  - Calls external executables (e.g., `phase1.exe` and `phase2.exe`) to assemble and simulate the code.
  - Parses and displays the assembled code, memory contents, and simulation results.

- **input.asm**  
  Example assembly file that includes both data (with directives like `.data` and `.word`) and text sections (the actual instructions).

- **input.mc**  
  The machine code file generated from the assembly source. It includes a text segment (with machine instructions) and a data segment (with initialized data).

- **memory.mc** and **Registers.mc**  
  Files produced by the simulator showing the final state of memory and registers after simulation.

- **Additional files**  
  - **bashrc**: Contains shell settings.
  - **format_output.txt** and **output.txt**: Logs and outputs from the simulation.
  - Other helper source files related to phase1 processing.

## Getting Started

### Prerequisites

- **C++ Compiler**: Ensure you have a C++ compiler installed to build the simulator.
- **Python 3**: Required to run the Flask web interface.
- **Flask**: Install using `pip install Flask`.
- External executables (e.g., `phase1.exe`, `phase2.exe`) should be built from the provided C++ source files.

### Running the Simulator

1. **Assemble and Simulate**  
   - Place your assembly code in the `input.asm` file.
   - Run the simulator executable (compiled from `Decode.cpp`) to process `input.mc` and produce output files (`memory.mc`, `Registers.mc`).

2. **Using the Web Interface**  
   - Run the web application with the following command:  
     ```
     python app.py
     ```
   - Open your web browser and navigate to the designated IP address (usually `http://127.0.0.1:5000/`).
   - Submit your assembly code via the provided form. The application will assemble the code, simulate its execution, and display the machine code, assembly, and memory contents.

## Notes

- The simulator currently supports a subset of the RISC-V instruction set (RV32) and may not support all instructions (for example, `ld` and `sd` are not supported for RV32).
- The web interface uses subprocess calls to external executables for assembly and simulation. Make sure these executables are in your system’s PATH or in the project directory.
- Memory addressing and register initialization are handled by predefined global variables in the simulator code.

