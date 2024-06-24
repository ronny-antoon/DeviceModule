idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS ""
                       REQUIRES)

# matter
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
# end matter