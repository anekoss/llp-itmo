cmake_minimum_required(VERSION 3.24)
project(lab1_llp C)

set(CMAKE_C_STANDARD 11)

add_executable(lab1_llp src/element.c src/element.c include/doc.h src/doc.c include/file.h src/file.c src/tests.c src/query.c src/operation.c src/list.c src/condition.c)
