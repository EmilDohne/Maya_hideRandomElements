import sys
import telnetlib

port = 20230

if len(sys.argv) > 1:
    port = sys.argv[1]
    
try:
    tn = telnetlib.Telnet("localhost", port)
    tn.write('file -o -f "C:/Users/emild/Desktop/test_scene_select_elements2.mb";'.encode())
    tn.write('catchQuiet(`loadPlugin "Maya_hideRandomElements"`);'.encode())
    tn.write('select pPlatonic21;'.encode())
    tn.write('HideRandomElements;'.encode())
    tn.close()
    
except:
    pass
