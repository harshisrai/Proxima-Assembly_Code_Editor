document.addEventListener("DOMContentLoaded", function () {
  const codeEditorTextarea = document.getElementById("code-editor");
  const editorContainer = document.getElementById("editor-container");
  const resizeHandle = document.getElementById("resize-handle");
  const runButton = document.getElementById("run-btn");
  const themeToggle = document.getElementById("theme-toggle");
  const lineCount = document.getElementById("line-count");
  const memoryViewer = document.getElementById("memory-viewer");
  const registerViewer = document.getElementById("register-viewer");
  const tabTriggers = document.querySelectorAll(".tab-trigger");
  const tabContents = document.querySelectorAll(".tab-content");

  let isRunning = false;
  let isResizing = false;
  let startX = 0;
  let startEditorWidth = 0;

  // Initialize CodeMirror
  const editor = CodeMirror.fromTextArea(codeEditorTextarea, {
    mode: "gas",
    theme: "dracula",
    lineNumbers: true,
    lint: true,
    lineWrapping: false,
    autoCloseBrackets: true,
    matchBrackets: true,
    styleActiveLine: true,
    foldGutter: true,
    gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
    tabSize: 4,
    indentWithTabs: true,
    extraKeys: {
      "Ctrl-/": "toggleComment",
      "Cmd-/": "toggleComment",
      Tab: (cm) => {
        if (cm.state.completionActive) {
          cm.execCommand("autocomplete");
        } else {
          cm.execCommand("indentMore");
        }
      },
      "Shift-Tab": (cm) => cm.execCommand("indentLess"),
      "Ctrl-Space": (cm) =>
        cm.showHint({
          hint: CodeMirror.hint.assembly,
          completeSingle: false,
        }),
    },
  });

  editor.on("cursorActivity", function () {
    document.getElementById("line-count").textContent =
      editor.getCursor().line + 1;
  });

  function getLabels(lastLine) {
    let labels = new Set();
    for (let i = 0; i <= lastLine; i++) {
      const temp_tokens = editor.getLine(i).trim().split(/\s+/);
      if (temp_tokens[0].endsWith(":")) {
        if (labels.has(temp_tokens[0].slice(0, -1)) == true) {
          //show error
          let label = temp_tokens[0].slice(0, -1);
          alert("Label '" + label + "' already exists");
        }
        labels.add(temp_tokens[0].slice(0, -1));
      }
    }
    console.log(labels);
    return labels;
  }
  function getCurrentSection(editor, currentLine) {
    let section = ".text"; // default
    for (let i = currentLine; i >= 0; i--) {
      const line = editor.getLine(i).trim();
      if (line.startsWith(".data")) {
        section = ".data";
        break;
      }
      if (line.startsWith(".text")) {
        section = ".text";
        break;
      }
    }
    return section;
  }

  // Add after editor initialization
  const assemblyHints = {
    instructions: [
      "add",
      "addi",
      "sub",
      "lui",
      "auipc",
      "jal",
      "jalr",
      "beq",
      "bne",
      "blt",
      "bge",
      "bltu",
      "bgeu",
      "lw",
      "sw",
      "sll",
      "srl",
      "sra",
      "xor",
      "ori",
      "andi",
      "ecall",
      "fence",
      "fence.i",
      "csrrw",
      "csrrs",
      "csrrc",
    ],
    registers: [
      "zero",
      "ra",
      "sp",
      "gp",
      "tp",
      "t0",
      "t1",
      "t2",
      "s0",
      "s1",
      "a0",
      "a1",
      "a2",
      "a3",
      "a4",
      "a5",
      "a6",
      "a7",
      "s2",
      "s3",
      "s4",
      "s5",
      "s6",
      "s7",
      "s8",
      "s9",
      "s10",
      "s11",
      "t3",
      "t4",
      "t5",
      "t6",
      "pc",
    ],
    dataDirectives: [".byte", ".half", ".word", ".dword", ".asciz"],
    directives: [
      ".text",
      ".data",
      ".globl",
      ".section",
      ".include",
      ".macro",
      ".endm",
    ],
  };

  // Custom hint function
  CodeMirror.registerHelper("hint", "assembly", (editor) => {
    const cur = editor.getCursor();
    const token = editor.getTokenAt(cur);
    const line = editor.getLine(cur.line);

    const start = token.start;
    const end = token.end;
    const currentWord = token.string.toLowerCase();

    // Determine context
    // const isDirective = line.trim().startsWith('.');
    // const isInstruction = /^\s*[a-zA-Z]/.test(line);

    // if (currentWord.length === 0) return null;
    // let hints = [];
    // if (isDirective) {
    //   hints = assemblyHints.directives;
    // } else if (isInstruction) {
    //   hints = assemblyHints.instructions;
    // } else {
    //   hints = [
    //     ...assemblyHints.registers,
    //     ...assemblyHints.instructions,
    //     ...assemblyHints.directives
    //   ];
    // }
    const currentSection = getCurrentSection(editor, cur.line);
    const trimmedLine = line.trim();
    //last line
    const lastline = editor.lastLine();
    const labels = getLabels(lastline);
    const isDirective = trimmedLine.startsWith(".");
    const tokens = trimmedLine.split(/\s+/);
    const firstToken = tokens[0] ? tokens[0].toLowerCase() : "";
    const isLabel = firstToken.endsWith(":");
    let push = false;
    const secondToken = tokens[1] ? tokens[1].toLowerCase() : "";
    let hints = [];

    console.log("label", isLabel);

    // Section-based logic
    if (currentSection === ".data") {
      hints = assemblyHints.dataDirectives;
      // No else - don't suggest anything for non-directives in .data
    } else {
      // .text section
      if (isDirective) {
        hints = assemblyHints.directives;
      } else {
        // Check if first token is a known instruction
        let isInstructionLine = true;
        if (
          assemblyHints.instructions.includes(firstToken) ||
          assemblyHints.instructions.includes(secondToken)
        ) {
          isInstructionLine = true;
        } else {
          isInstructionLine = false;
        }
        if (isInstructionLine) {
          // After instruction, show only register
          let labels_ = [...labels];
          hints = [assemblyHints.registers, labels_].flat();
        } else {
          // First token or unknown instruction - show instructions
          hints = assemblyHints.instructions;
        }
      }
    }

    const filtered = hints
      .filter((hint) =>
        hint.toLowerCase().startsWith(currentWord.toLowerCase())
      )
      .sort();

    return filtered.length > 0
      ? {
          list: filtered,
          from: CodeMirror.Pos(cur.line, start),
          to: CodeMirror.Pos(cur.line, end),
        }
      : null;
  });

  // Enable autocomplete on typing
  editor.on("inputRead", (cm, input) => {
    // suggest only when string length is greater than 1  and first character is alphabet
    const curWord = cm.getTokenAt(cm.getCursor()).string;
    if (curWord.length > 0 && input.text && input.text[0].match(/[a-zA-Z\.]/)) {
      cm.showHint({
        hint: CodeMirror.hint.assembly,
        completeSingle: false,
        // completeSingle: false,
        // alignWithWord: true
      });
    }
  });

  editor.on("change", () => {
    lineCount.textContent = editor.getValue().split("\n").length;
  });

  // Theme toggle handler
  themeToggle.addEventListener("click", () => {
    const isDark = document.body.classList.toggle("dark");
    editor.setOption("theme", isDark ? "dracula" : "solarized light");
    themeToggle.querySelector(".icon").textContent = isDark ? "🌞" : "🌙";
    //change font color of stage-content id to black
    let stageContent = document.querySelectorAll(".stage-content");
    // stageContent.style.fontColor = isDark ? "#d4d4d4" : "black";
    stageContent.forEach((element) => {
      element.style.color = isDark ? "#d4d4d4" : "black";
    });
  });

  // Tab switching handler
  tabTriggers.forEach((trigger) => {
    trigger.addEventListener("click", () => {
      const tab = trigger.dataset.tab;
      tabTriggers.forEach((t) => t.classList.remove("active"));
      trigger.classList.add("active");
      tabContents.forEach((content) => content.classList.remove("active"));
      document
        .querySelector(`.tab-content[data-tab="${tab}"]`)
        .classList.add("active");
    });
  });

  // Resize handle handler
  resizeHandle.addEventListener("mousedown", (e) => {
    isResizing = true;
    startX = e.clientX;
    startEditorWidth = editorContainer.offsetWidth;
    document.body.style.cursor = "col-resize";
    document.body.style.userSelect = "none";
  });

  document.addEventListener("mousemove", (e) => {
    if (!isResizing) return;
    const containerWidth = editorContainer.parentElement.offsetWidth;
    const deltaX = e.clientX - startX;
    const newWidth = Math.max(
      300,
      Math.min(containerWidth - 300, startEditorWidth + deltaX)
    );
    editorContainer.style.width = `${newWidth}px`;
  });

  document.addEventListener("mouseup", () => {
    isResizing = false;
    document.body.style.cursor = "";
    document.body.style.userSelect = "";
  });

  const registerNames = [
    "zero",
    "ra",
    "sp",
    "gp",
    "tp",
    "t0",
    "t1",
    "t2",
    "s0/fp",
    "s1",
    "a0",
    "a1",
    "a2",
    "a3",
    "a4",
    "a5",
    "a6",
    "a7",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "s8",
    "s9",
    "s10",
    "s11",
    "t3",
    "t4",
    "t5",
    "t6",
  ];

  // Initialize memory and register viewers
  const sampleMemory = Array(16)
    .fill(0)
    .map((_, i) => ({
      address: `0x${(i * 4).toString(16).padStart(8, "0")}`,
      value: `0x${Math.floor(Math.random() * 0xffffffff)
        .toString(16)
        .padStart(8, "0")}`,
    }));

  const sampleRegisters = registerNames.map((name, i) => ({
    name: `x${i} (${name})`,
    value: `0x${Math.floor(Math.random() * 0xffffffff)
      .toString(16)
      .padStart(8, "0")}`,
  }));

  //   memoryViewer.innerHTML = `
  //     <div class="flex flex-col h-full">
  //         <div class="p-3 border-b border-border flex items-center gap-2 bg-card">
  //             <input type="text" placeholder="Search..." class="search-input flex-1 px-3 py-1.5 text-sm rounded border border-border">
  //             <button class="btn-outline px-3 py-1.5 text-sm">
  //                 Search
  //             </button>
  //         </div>
  //         <div class="memory-table flex-1 overflow-auto">
  //             ${sampleMemory
  //               .map(
  //                 (mem) => `
  //                 <div class="memory-row hover:bg-muted/50 transition-colors">
  //                     <div class="memory-address">${mem.address}</div>
  //                     <div class="memory-value">${mem.value}</div>
  //                 </div>
  //             `
  //               )
  //               .join("")}
  //         </div>
  //     </div>
  // `;

  //   // Update register viewer rendering
  //   registerViewer.innerHTML = `
  //     <div class="flex flex-col h-full">
  //         <div class="p-3 border-b border-border bg-card">
  //             <h3 class="text-sm font-medium">Register State</h3>
  //         </div>
  //         <div class="register-grid flex-1 overflow-auto p-3">
  //             ${sampleRegisters
  //               .map(
  //                 (reg) => `
  //                 <div class="register-item hover:border-primary/30 transition-colors">
  //                     <div class="register-name">${reg.name}</div>
  //                     <div class="register-value">${reg.value}</div>
  //                 </div>
  //             `
  //               )
  //               .join("")}
  //         </div>
  //     </div>
  // `;

  document.getElementById("importBtn").addEventListener("click", function () {
    document.getElementById("fileInput").click();
  });

  document
    .getElementById("fileInput")
    .addEventListener("change", function (event) {
      const file = event.target.files[0];
      if (!file) return;
      const reader = new FileReader();

      reader.onload = function (e) {
        editor.setValue(e.target.result);
      };

      reader.onerror = function () {
        alert("Error reading the file. Please try again.");
      };

      reader.readAsText(file);
    });

  let butto = document.getElementById("saveBtn");

  // eventListener "click" on button
  butto.addEventListener("click", () => {
    let valueinput = editor.getValue();
    console.log(valueinput);
    let blobdtMIME = new Blob([valueinput], { type: "text/plain" });
    let url = URL.createObjectURL(blobdtMIME);
    let anele = document.createElement("a");
    let inputFileName = prompt("Enter file name");
    anele.setAttribute("download", inputFileName);
    anele.href = url;
    anele.click();
    console.log(blobdtMIME);
  });
});
