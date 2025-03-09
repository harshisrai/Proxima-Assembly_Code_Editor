# Assembly Code Processing Project

## Overview
This project processes an assembly file (`input.asm`) using `main.cpp`. It generates two auxiliary files: `refined_code.asm` and `output.txt`, with the final output stored in `output.mc`. Additional test cases are available in the `inputs/` folder.

## File Structure
```
.
├── inputs/             # Additional input cases
│   ├── input1.asm
│   ├── input2.asm
│   ├── input3.asm
│   ├── input4.asm
├── input.asm          # Main input file
├── main.cpp           # C++ program to process assembly file
├── main.exe         # Compiled executable
├── output.mc          # Final processed output
├── output.txt         # Intermediate output file
├── refined_code.asm   # Refined assembly code
├── README.md          # Project documentation
```

## Usage
### Compilation
Compile the `main.cpp` file using g++:
```sh
g++ main.cpp -o merged.exe
```

### Running the Program
Run the compiled executable:
```sh
./merged.exe
```
By default, it takes `input.asm` and generates `refined_code.asm`, `output.txt`, and the final output `output.mc`.

## Additional Inputs
For additional test cases, place the respective `.asm` files inside the `inputs/` folder and modify the program to process them accordingly.

## Requirements
- C++ Compiler (g++)
- Assembly knowledge for modifying inputs

