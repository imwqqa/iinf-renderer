set(SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/triangle.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/triangle.frag"
)

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/src/shaders")

find_program(GLSLC glslc REQUIRED)

set(SPV_OUTPUTS "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPV_FILE "${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv")
    list(APPEND SPV_OUTPUTS ${SPV_FILE})

    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
        COMMAND ${GLSLC} ${SHADER} -o ${SPV_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling : ${FILE_NAME}"
        VERBATIM
    )
endforeach()

add_custom_target(compile_shaders DEPENDS ${SPV_OUTPUTS})
add_dependencies(iinf-app compile_shaders)
