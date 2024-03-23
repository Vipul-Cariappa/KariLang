cmake_minimum_required(VERSION 3.20)
project(KariLang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-Wall)

find_package(BISON 3.0 REQUIRED)

bison_target(PARSER "src/Parser.yy" "${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.cc" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.hh" COMPILE_FLAGS "-Wall -Wcounterexamples")

find_package(FLEX 2.0 REQUIRED)

flex_target(LEXER "src/Lexer.l" "${CMAKE_CURRENT_BINARY_DIR}/Lex.yy.cc")

find_package(LLVM 17 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        src/
        ${LLVM_INCLUDE_DIRS}
        )

add_definitions(${LLVM_DEFINITIONS_LIST})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

add_executable(KariLang src/Main.cc
        src/AST.cc
        src/Compile.cc
        src/JIT.cc
        src/AST.hh
        src/Compile.hh
        src/JIT.hh
        src/Parser.hh
        src/Utils.hh
        ${BISON_PARSER_OUTPUTS}
        ${FLEX_LEXER_OUTPUTS})

# getting LLVM libraries to link with
set(KARILANG_LLVM_COMPONENTS core support mcjit orcjit native asmparser asmprinter irreader)


# Enable the native target
if ("${LLVM_NATIVE_ARCH}" STREQUAL "AArch64")
        set(WITH_TARGET_AARCH64 yes)
endif()

if ("${LLVM_NATIVE_ARCH}" STREQUAL "X86")
        set(WITH_TARGET_X86 yes)
endif()

if (WITH_TARGET_AARCH64)
        if (NOT ("${LLVM_TARGETS_TO_BUILD}" MATCHES "AArch64"))
            message(FATAL_ERROR "The selected LLVM library doesn't have support for AArch64 targets")
        endif()

        list(APPEND KARILANG_LLVM_COMPONENTS aarch64info aarch64utils aarch64desc aarch64asmparser aarch64codegen aarch64disassembler)
        add_definitions("-DHAVE_TARGET_AARCH64=1")
endif()

if (WITH_TARGET_X86)
        if (NOT ("${LLVM_TARGETS_TO_BUILD}" MATCHES "X86"))
                message(FATAL_ERROR "The selected LLVM library doesn't have support for X86 targets")
        endif()

        list(APPEND KARILANG_LLVM_COMPONENTS x86info x86desc x86codegen x86asmparser x86disassembler)
        add_definitions("-DHAVE_TARGET_X86=1")
endif()

llvm_map_components_to_libnames(llvm_libs ${KARILANG_LLVM_COMPONENTS})

target_link_libraries(KariLang
        "${llvm_libs}")
