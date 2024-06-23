cmake_minimum_required(VERSION 3.16)

file(GLOB SRC_FILES "src/*.cpp")

idf_component_register(SRCS "${SRC_FILES}"
                       INCLUDE_DIRS "include"
                       REQUIRES 
                       PRIV_REQUIRES)