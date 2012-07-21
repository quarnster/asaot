cmake_minimum_required(VERSION 2.8)

set(ASPATH "~/code/3rdparty/angelscript/sdk/angelscript/")

add_custom_command(
    OUTPUT generated.cpp
    COMMAND python ${PROJECT_SOURCE_DIR}/getbytecodes.py ${ASPATH} > generated.cpp
    DEPENDS ${PROJECT_SOURCE_DIR}/getbytecodes.py
     )

include_directories(${ASPATH}/include ${ASPATH}/source .)

add_library(asaot generated.cpp)
