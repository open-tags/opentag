idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "src")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")