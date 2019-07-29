project(CJson)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
aux_source_directory(. source)
add_library(${PROJECT_NAME} STATIC ${source})
