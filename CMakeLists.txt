cmake_minimum_required(VERSION 2.8)

find_package(PythonInterp)

set(ASPATH "${PROJECT_SOURCE_DIR}/../3rdparty/angelscript/sdk/angelscript/" CACHE PATH "Directory containing angelscript")

if(NOT PYTHON_EXECUTABLE)
    message("Warning: Python executable not found. Going to try using the 'python' command anyway")
    set(PYTHON_EXECUTABLE python)
endif()

add_custom_command(
    OUTPUT generated.cpp
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/getbytecodes.py ${ASPATH} > generated.cpp
    DEPENDS ${PROJECT_SOURCE_DIR}/getbytecodes.py
     )

add_custom_command(
    OUTPUT test2.cpp
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/generate.py ${ASPATH} > test2.cpp
    DEPENDS ${PROJECT_SOURCE_DIR}/tests/generate.py
     )

if(NOT EXISTS ${PROJECT_BINARY_DIR}/aot_generated_code.cpp)
    file(WRITE ${PROJECT_BINARY_DIR}/aot_generated_code.cpp "")
endif()
if(NOT EXISTS ${PROJECT_BINARY_DIR}/aot_generated_code2.cpp)
    file(WRITE ${PROJECT_BINARY_DIR}/aot_generated_code2.cpp "")
endif()

link_directories(${ASPATH}/lib)
include_directories(${ASPATH}/include ${ASPATH}/source ${ASPATH}/../add_on include)

add_library(asaot source/AOTCompiler.cpp source/AOTFunction.cpp generated.cpp)

add_executable(test tests/test.cpp ${PROJECT_BINARY_DIR}/aot_generated_code.cpp ${ASPATH}/../add_on/scriptstdstring/scriptstdstring.cpp)
target_link_libraries(test asaot Angelscript)


add_executable(test2 test2.cpp ${PROJECT_BINARY_DIR}/aot_generated_code2.cpp)
target_link_libraries(test2 asaot Angelscript)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT ANDROID)
    target_link_libraries(test2 pthread)
    target_link_libraries(test pthread)
endif()

