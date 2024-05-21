idf_component_register(SRCS "hello_world_main.cpp"
                    INCLUDE_DIRS "lib/dwt_uwb_driver/Inc")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")