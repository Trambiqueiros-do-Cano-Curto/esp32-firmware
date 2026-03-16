FILE(GLOB MAIN_FILES src/*/*.cpp)

idf_component_register(
    SRCS
        "src/main.cpp"
        ${MAIN_FILES}

    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_wifi 
)