cmake_minimum_required(VERSION 2.8)

set(ASPATH "/Users/quarnster/code/3rdparty/angelscript/sdk/angelscript/")

add_custom_command(
    OUTPUT generated.cpp
    COMMAND python ${PROJECT_SOURCE_DIR}/getbytecodes.py ${ASPATH} > generated.cpp
    DEPENDS ${PROJECT_SOURCE_DIR}/getbytecodes.py
     )

include_directories(${ASPATH}/include ${ASPATH}/source .)

add_library(asaot AOTCompiler.cpp AOTFunction.cpp generated.cpp)


add_executable(test test.cpp)
target_link_libraries(test -L${ASPATH}/lib asaot Angelscript)
