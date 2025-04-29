from flask import Flask, request, render_template, Response
import subprocess
import re
from flask_socketio import SocketIO, emit
import subprocess, threading
import signal

app = Flask(__name__)
socketio = SocketIO(app)
user_code = ""
table_data = []
memory = []
memoryDict = {}
proc = None

@app.route("/", methods=["GET", "POST"])
def index():
    global user_code 
    global table_data
    global memory 
    global memoryDict
    
    # user_code = ""
    # table_data = []
    # memory = []
    # memoryDict = {}
    if request.method == "POST":
        user_code = ""
        table_data = []
        memory = []
        memoryDict = {}
        # Retrieve code submitted by the user
        user_code = request.form.get("code")
        print(user_code)
        # Check if user_code is None or empty
        if user_code is None or user_code.strip() == "":
            table_data = [
                {
                    "address": "No code provided",
                    "machine_code": "No code provided",
                    "assembly_code": "No code provided",
                }
            ]
            # return render_template("index.html", code=user_code, table_data=table_data, memory=memory)

        # Save the code to a file with a .mc extension
        file_path = "input.asm"
        try:
            user_code = user_code.replace("\r\n", "\n")
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(user_code)
        except Exception as e:
            return f"Error writing file: {e}", 500

        try:
            process = subprocess.Popen(
                ["phase1.exe"],
            )
            process.wait()
            # load content of file name refined_code.asm in assembled_code
            with open("input.mc", "r") as f:
                assembled_code = f.read()
                lines = assembled_code.strip().split("\n")

                for line in lines:
                    print(line)
                    match = re.match(r"^(\S+):\s+(\S+)\s+(.*?)#\s*(.*)$", line)
                    if match:
                        address = match.group(1)  # "0x0"
                        machine_code = match.group(2)  # "0x033302b3"
                        assembly_code = match.group(3).strip()  # "add x1, x2, x3"
                        table_data.append(
                            {
                                "address": address,
                                "machine_code": machine_code,
                                "assembly_code": assembly_code,
                            }
                        )
                    # other format of line : 0x10000000     0x00000001
                    match = re.match(r"^(\S+)\s+(\S+)$", line)
                    if match:
                        address = match.group(1)
                        value = match.group(2)
                        value = value[2:]
                        # if the address isn't multiple of 4, find the greatest multiple of 4 less than the address and add 0s and load to that address
                        address = int(address, 16)
                        if address % 4 != 0:
                            rem = address % 4
                            address = address - rem
                            print("before: ", value)
                            # value should have (address%4)*2 0s added at the right end
                            print("address%4: ", rem)
                            value = value + "0" * (rem) * 2
                            print("after: ", value)
                            # make it 8 digit hex
                            value = value.zfill(8)
                            address = hex(address)
                        else:
                            value = value.zfill(8)
                            address = hex(address)
                        # memoryDict[address] =  memoryDict[address] + value (Arithmetic operation)
                        if address in memoryDict:
                            print("Address already exists")
                            print(memoryDict[address])
                            print(value)
                            print("\n\n")
                            memoryDict[address] = hex(
                                int(memoryDict[address], 16) + int(value, 16)
                            )
                            # make it 8 digit hex
                            memoryDict[address] = memoryDict[address][2:].zfill(8)
                        else:
                            memoryDict[address] = value

                print(memoryDict)
                # convert memory dictionary to list of dictionaries
                for key in memoryDict:
                    memory.append(
                        {
                            "address": key,
                            "value": "  ".join(
                                memoryDict[key][i : i + 2]
                                for i in range(0, len(memoryDict[key]), 2)
                            ),
                        }
                    )
                print("Table Data")
                print(table_data)
                print("Memory")
                print(memory)
        except Exception as e:
            return f"Error starting phase.exe: {e}"

        return render_template(
            "index.html", code=user_code, table_data=table_data, memory=memory
        )

    # Render the index.html template for GET requests
    return render_template(
        "index.html", code=user_code, table_data=table_data, memory=memory
    )

@app.route("/pipelined", methods=["GET", "POST"])
def pipeline():
    return render_template(
        "pipelined.html", table_data=table_data
    )
    

@app.route("/stream")
def stream():
    return Response(stream_output(), mimetype="text/plain")

def reader_thread():
    global user_pause
    """ Continuously read lines from the C++ process and emit to client """
    global proc
    for line in proc.stdout:
        # print(line)
        socketio.emit('console_output', {'line': line})
    proc.stdout.close()

import os
import signal

@socketio.on('start_cpp')
def on_start(data):
    global proc
    # 1) If there’s an existing process, shut it down
    print("here")
    if proc is not None:
        try:
            print("trying to end")
            # send CTRL+BREAK to the process group
            proc.send_signal(signal.CTRL_BREAK_EVENT)
            proc.wait(timeout=5)
        except Exception as e:
            print(f"Graceful shutdown failed: {e}, forcing kill")
            proc.kill()
            proc.wait()
        finally:
            proc = None

    # 2) Start a fresh process in its own group
    proc = subprocess.Popen(
        ['./phase3'], 
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1,
        creationflags=subprocess.CREATE_NEW_PROCESS_GROUP
    )

    # 3) Send initial parameters
    proc.stdin.write(f"{data['cycle_time']}\n{data['step_mode']}\n")
    proc.stdin.flush()

    # 4) Spin up your reader thread again
    t = threading.Thread(target=reader_thread, daemon=True)
    t.start()


@socketio.on('user_command')
def on_command(data):
    """
    data = { 'cmd': 0 } or { 'cmd': 1 }
    this is sent whenever user clicks “next step” after a pause
    """
    # print("I got called")
    global proc
    global user_pause
    if proc:
        print("data: ", data['cmd'])
        user_pause=data['cmd']
        proc.stdin.write(f"{data['cmd']}\n")
        proc.stdin.flush()
    
    
def stream_output():

    # Launch sample.exe that reads the .mc file and outputs continuously
    try:
        process2 = subprocess.Popen(
            ["phase2.exe"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        # Yield each line of output as soon as it is available
        for line in iter(process2.stdout.readline, ""):
            yield line
        process2.stdout.close()
        process2.wait()
    except Exception as e:
        yield f"Error starting phase2.exe: {e}"
        return
    


if __name__ == "__main__":
    app.run(debug=True)
