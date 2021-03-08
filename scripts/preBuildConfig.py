import json
import binascii

import inspect, os.path

def preBuildConfigFun():
    filename = inspect.getframeinfo(inspect.currentframe()).filename
    dir_path = os.path.dirname(os.path.abspath(filename))

    filename = "config"
    h = open(dir_path + "/../src/generated/" + filename + ".h", "w", encoding="utf8")
    cpp = open(dir_path + "/../src/generated/" + filename + ".cpp", "w", encoding="utf8")

    with open(dir_path + '/../gui/js/configuration.json') as f:
        data = json.load(f)

    # binascii.crc32(mes.encode('utf8'))
    #headers
    h.write("#pragma once\n\n")
    h.write("struct ConfigData {\n")

    cpp.write("#include <Arduino.h>\n")
    cpp.write("#include \"config.h\"\n\n")

    cpp.write("const ConfigData configDefaults PROGMEM = {\n")

    #loop through variables
    first = True
    for item in data: 
        
        if first==True:
            first=False
        else:
            cpp.write(',\n')

        if item['type'] == 'char':
            cpp.write("\t\"" + item['value'] + "\"")
            h.write("\tchar " + item['name'] + "[" + str(item['length']) + "];\n")
        elif item['type'] == 'bool':
            cpp.write("\t" + str(item['value']).lower())
            h.write("\t" + item['type'] + " " + item['name'] +";\n")
        else:
            cpp.write("\t" + str(item['value']))
            h.write("\t" + item['type'] + " " + item['name'] +";\n")

    #footers

    h.write("};\n\nconstexpr uint32_t configVersion = " + str(binascii.crc32(json.dumps(data).encode())) + "; //generated identifier to compare config with EEPROM\n\n")
    h.write("extern const ConfigData configDefaults;\n")

    cpp.write("\n};\n")

    h.close()
    cpp.close()
