
import json

opcodes_json = open("Opcodes.json")

opcodes_data = json.load(opcodes_json)

out = open("opcode_table.h", "w")
out.write("#ifndef GBEX_OPCODE_TABLE\n")
out.write("#define GBEX_OPCODE_TABLE\n\n\n")

out.write("#include <cstdint>\n\n")

out.write("namespace gbex\n{\n\n")

out.write("\tstruct Instruction\n\t{\n")
out.write("\t\tconst char* mnemonic;\n");
out.write("\t\tuint8_t bytes;\n");
out.write("\t\tuint8_t cycles;\n");
out.write("\t};\n\n");

out.write("\tstatic Instruction instructions[] = {\n");

for i in opcodes_data["unprefixed"]:
    op = opcodes_data["unprefixed"][i]
    out.write("\t\t{")

    # handle mnemonic
    out.write(" \"" + op["mnemonic"])
    op_count = 0
    for operand in op["operands"]:
        if op_count != 0:
            out.write(", ")
        else:
            out.write(" ")
        out.write(operand["name"])
        op_count += 1
    out.write("\", ")

    # Handle byte size and cycles
    out.write(str(op["bytes"]) + ", ")
    
    if len(op["cycles"]) == 1:
        out.write(str(op["cycles"][0]))
    else:
        out.write("0")

    out.write(" },\n")

out.write("\t};\n\n")

out.write("\tstatic Instruction prefixed_instructions[] = {\n");

for i in opcodes_data["cbprefixed"]:
    op = opcodes_data["cbprefixed"][i]
    out.write("\t\t{")

    # handle mnemonic
    out.write(" \"" + op["mnemonic"])
    op_count = 0
    for operand in op["operands"]:
        if op_count != 0:
            out.write(", ")
        else:
            out.write(" ")
        out.write(operand["name"])
        op_count += 1
    out.write("\", ")

    # Handle byte size and cycles
    out.write(str(op["bytes"]) + ", ")
    
    if len(op["cycles"]) == 1:
        out.write(str(op["cycles"][0]))
    else:
        out.write("0")

    out.write(" },\n")

out.write("\t};\n")

out.write("\n}")

out.write("\n\n#endif // GBEX_OPCODE_TABLE")

opcodes_json.close()