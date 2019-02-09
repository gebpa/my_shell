cmake_minimum_required(VERSION 3.13)
project(lab4 C)

set(CMAKE_C_STANDARD 99)

add_executable(lab4 main.c variables.c my_shell.h exec_built-in.c)