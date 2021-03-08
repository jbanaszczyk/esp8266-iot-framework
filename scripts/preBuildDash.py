import json
import binascii

import inspect, os.path

def preBuildDashFun():
    filename = inspect.getframeinfo(inspect.currentframe()).filename
    dir_path = os.path.dirname(os.path.abspath(filename))

    filename = "dash"
    h = open(dir_path + "/../src/generated/" + filename + ".h", "w", encoding="utf8")

    with open(dir_path + '/../gui/js/dashboard.json') as f:
        data = json.load(f)

    # binascii.crc32(mes.encode('utf8'))
    #headers
    h.write("#pragma once\n\n")
    h.write("struct DashboardData {\n")

    #loop through variables
    for item in data:        
        if item['type'] == 'char':
            h.write("\tchar " + item['name'] + "[" + str(item['length']) + "];\n")
        elif item['type'] == 'bool':
            h.write("\t" + item['type'] + " " + item['name'] +";\n")
        else:
            h.write("\t" + item['type'] + " " + item['name'] +";\n")

    #footers    
    h.write("};\n")

    h.close()
