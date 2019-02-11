cmake_minimum_required(VERSION 3.12)
project(VM C)

set(CMAKE_C_STANDARD 11)

add_executable(VM stack_vm.c)