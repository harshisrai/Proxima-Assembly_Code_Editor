from flask import Flask, request, render_template, Response
import subprocess
import re

app = Flask(__name__)


@app.route("/", methods=["GET", "POST"])
def index():
    user_code = ""
    table_data = []
    memory = []
    memoryDict = {}
    if request.method == "POST":
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
            return render_template("index.html", code=user_code, table_data=table_data)

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


@app.route("/stream")
def stream():
    return Response(stream_output(), mimetype="text/plain")


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
