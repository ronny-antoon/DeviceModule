cmake_minimum_required(VERSION 3.16)

# matter
set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)
# end matter

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(component_test)

# matter
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# For RISCV chips, project_include.cmake sets -Wno-format, but does not clear various
# flags that depend on -Wformat
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
# end matter