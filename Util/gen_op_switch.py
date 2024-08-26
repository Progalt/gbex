
import json

opcodes_json = open("Opcodes.json")

opcodes_data = json.load(opcodes_json)

out = open("opcode_switch.cpp", "w")

def output_switch(opcode_array):
    out.write("switch(opcode)\n{\n")


    for i in opcodes_data[opcode_array]:
        op = opcodes_data[opcode_array][i]
        out.write("case " + str(i) + ":")
        out.write("\t\t\t// ")

        # handle mnemonic
        out.write(op["mnemonic"])
        op_count = 0
        for operand in op["operands"]:
            if op_count != 0:
                out.write(", ")
            else:
                out.write(" ")
            out.write(operand["name"])
            op_count += 1

        out.write("\n\tthrow std::runtime_error(\"Unimplemented CB Instruction: " + str(i) + "\");")
        out.write("\n\tbreak;\n")

    out.write("\n}")



output_switch("cbprefixed")

out.close()