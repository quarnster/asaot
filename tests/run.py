import re
import os
import platform
import time

def replace(a, b):
    f = open("test2.cpp")
    data = f.read().replace("CODE %d" % a, "CODE %d" % b)
    f.close()
    f = open("test2.cpp", "w")
    f.write(data)
    f.close()

def dotest(config=""):
    make = "make test2 && ./test2"
    if platform.system() == "Windows":
        make = "nmake test2 && test2"

    os.system("cmake %s .. && cmake -E remove -f test2*" % config)
    os.system(make)
    time.sleep(2) # hack just to give the files a different modification timestamp
    os.utime("aot_generated_code2.cpp", None)
    replace(1, 0)
    os.system(make)

os.chdir("../build")
if platform.system() == "Darwin":
    os.system("rm -rf *")
    dotest("-DCMAKE_OSX_ARCHITECTURES=x86_64")
    os.system("rm -rf *")
    dotest("-DCMAKE_OSX_ARCHITECTURES=i386")
else:
    os.system("cmake -E remove_directory .")
    dotest("-G \"NMake Makefiles\" -DASPATH=\"../../3rdparty/angelscript/sdk/angelscript\"")
