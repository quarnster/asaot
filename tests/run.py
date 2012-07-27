import re
import os

def dotest(config):
    os.system("cmake %s .. && rm -f test2* && make && ./test2" % config)
    f = open("test2.cpp")
    data = f.read().replace("CODE 1", "CODE 0")
    f.close()
    f = open("test2.cpp", "w")
    f.write(data)
    f.close()
    os.system("make && ./test2")

os.chdir("../build")
dotest("-DCMAKE_OSX_ARCHITECTURES=x86_64")
dotest("-DCMAKE_OSX_ARCHITECTURES=i386")
