# 🛰️ Proxima Studio – RISC-V Assembly Editor & Pipeline Simulator

## Team Members
- **Harsh Rai** (2023CSB1345)  
- **Nishant Sahni** (2023CSB1140)  
- **Hardik Garg** (2023CSB1121)
## Overview
Proxima Studio is an educational tool for writing, assembling, and simulating RISC-V assembly programs. It combines a powerful text editor, assembler, and an interactive pipeline visualizer into a seamless interface built for learning low-level systems programming.

This project processes an assembly file (`input.asm`) using `main.cpp`. It generates two auxiliary files: `refined_code.asm` and `output.txt`, with the final output stored in `output.mc`. Additional test cases are available in the `inputs/` folder.

## UI/UX 
### Code Editor 
![Code Editor](ui_pics/code_editor.png)

This is the heart of Proxima Studio’s user experience: a clean, responsive RISC-V assembly editor with syntax highlighting and inline auto-suggestions. On the left, users can seamlessly write or modify .s files, organizing .data and .text sections naturally. On the right, the real-time assembler displays memory-aligned machine code, decoded instructions, and instruction addresses in a synchronized table format. This immediate visual feedback empowers users to understand how their assembly maps to raw binary, bridging low-level code and hardware logic interactively.

### Pipeline Visualizer

![Pipeline Visualizer](ui_pics/pipeline.png)
Proxima Studio’s cycle-accurate RISC-V pipeline simulation in action. It vividly represents the classic 5-stage pipeline (Fetch, Decode, Execute, Memory, Write Back) with color-coded boxes that track each instruction as it flows through the processor. Users can observe instruction-level parallelism, identify hazards, and control execution step-by-step or in full-run mode. Additional toggles like pipelining, data forwarding, and register visibility enrich the debugging experience, making the learning process both intuitive and insightful.

---

## 📁 Folder & File Structure

### 🧾 Key Files

| File               | Description                                                                                                                 |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------- |
| `input.asm`        | ✍️ User-written RISC-V assembly code with optional comments.                                                                |
| `refined_code.asm` | 🧼 Cleaned version of `input.asm` with comments and whitespace removed — ready for machine code conversion.                 |
| `phase1.cpp`       | 🔧 Initial stage: converts clean RISC-V code into machine code. (Kept for reference from early development.)                |
| `phase3.cpp`       | 🧠 Main engine: simulates the RISC-V pipeline with runtime toggles for pipelining, data forwarding, branch prediction, etc. |
| `phase3.exe`       | 🚀 Executable version of the pipeline simulator.                                                                            |
| `output.txt`       | 📄 Output of the executed pipeline simulation — includes register/memory states or instruction traces.                      |
| `phase3_stats.txt` | 📊 Performance stats such as stalls, cycles, and CPI collected during execution.                                            |

---

### 📂 Folder Descriptions

| Folder       | Contents                                                                                                            |
| ------------ | ------------------------------------------------------------------------------------------------------------------- |
| `Testcases/` | ✅ Assembly test cases to be loaded into the editor for simulation and validation.                                   |
| `templates/` | 📑 Optional reusable boilerplate code or UI content.                                                                |
| `ui_pics/`   | 🖼️ Screenshots used for documentation (e.g., `code_editor.png`, `pipeline.png`).                                   |
| `static/`    | 📁 Static assets needed for frontend or visualization.                                                              |
| `phase1/`    | 🧪 First-stage prototype of converting RISC-V assembly to machine code — retained for comparison with later stages. |

---

## 🔄 Workflow

1. **User writes RISC-V assembly code** directly in the web-based code editor.
2. The system automatically generates `refined_code.asm` by removing comments and unnecessary whitespace.
3. **Assemble Step**: The refined code is translated into machine code using either `phase1.cpp` (initial prototype) or the more complete `phase3.cpp`.
4. **Simulate Step**: `phase3.cpp` runs the machine code through a configurable pipeline simulator, producing output in `output.txt` and performance metrics in `phase3_stats.txt`.
5. The simulation results are displayed in the UI, complete with stage-by-stage instruction flow and toggles for features like pipelining, forwarding, and branch prediction.

---

## 🧪 Testing

To verify functionality:

* Load any file from the `Testcases/` folder.
* Run through the simulator to inspect execution trace, stalls, and correctness of output.

---

## 📌 Note

The `phase1/` folder reflects our initial implementation and only handles the conversion to machine code. As the project matured, we transitioned to `phase3.cpp` for full simulation features including configurable pipeline behavior.

---

Let me know if you'd like badges, setup instructions, or usage examples added!
