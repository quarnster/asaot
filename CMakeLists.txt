cmake_minimum_required(VERSION 2.8)

set(ASPATH "/Users/quarnster/code/3rdparty/angelscript/sdk/angelscript/")

add_custom_command(
    OUTPUT generated.cpp
    COMMAND python ${PROJECT_SOURCE_DIR}/getbytecodes.py ${ASPATH} > generated.cpp
    DEPENDS ${PROJECT_SOURCE_DIR}/getbytecodes.py
     )

if(NOT EXISTS ${PROJECT_BINARY_DIR}/aot_generated_code.cpp)
    file(WRITE ${PROJECT_BINARY_DIR}/aot_generated_code.cpp "")
endif()

include_directories(${ASPATH}/include ${ASPATH}/source ${ASPATH}/../add_on .)

add_library(asaot AOTCompiler.cpp AOTFunction.cpp generated.cpp)


add_executable(test test.cpp ${PROJECT_BINARY_DIR}/aot_generated_code.cpp ${ASPATH}/../add_on/scriptstdstring/scriptstdstring.cpp)
target_link_libraries(test -L${ASPATH}/lib asaot Angelscript)

