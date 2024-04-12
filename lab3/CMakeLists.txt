cmake_minimum_required(VERSION 3.24)
project(lab3_llp C)

set(CMAKE_C_STANDARD 99)

add_executable(lab3_llp simple.pb.c simple.c  nanopb/pb_encode.c nanopb/pb_decode.c nanopb/pb_common.c ast.c)
