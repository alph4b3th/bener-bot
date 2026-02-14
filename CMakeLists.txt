cmake_minimum_required(VERSION 3.10)
project(bener_bot C)

find_package(CURL REQUIRED)

add_subdirectory(libollama)

add_executable(meu_programa
    src/main.c
    src/ollama_client.c
    src/json_utils.c
)


target_include_directories(meu_programa PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libollama/lib/public
)

target_link_libraries(meu_programa PRIVATE
    ollama
    CURL::libcurl
    cjson
)
