find_program(FXC_EXECUTABLE NAMES fxc.exe fxc)
if(NOT FXC_EXECUTABLE)
  message(FATAL_ERROR "fxc.exe was not found. " "Please install the DirectX SDK / Windows SDK or add fxc.exe to PATH." )
endif()
message(STATUS "Using fxc: ${FXC_EXECUTABLE}")
set(SHADER_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${SHADER_INCLUDE_DIR}")

file(GLOB_RECURSE SHADER_FILES
  CONFIGURE_DEPENDS 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.fx"
  # "${CMAKE_CURRENT_SOURCE_DIR}/*.hlsl"
)
foreach(SHADER_FILE IN LISTS SHADER_FILES)
  get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
  set(HEADER_FILE "${SHADER_INCLUDE_DIR}/${SHADER_NAME}.h")
  
  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" SHADER_VAR "${SHADER_NAME}")
  if(SHADER_VAR MATCHES "^[0-9]")
    set(SHADER_VAR "_${SHADER_VAR}")
  endif()

  # comiple to binary
  execute_process(
	  COMMAND "${FXC_EXECUTABLE}"
	    /T fx_2_0 
	    /Fo "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.fxo"
		/Fh "${SHADER_INCLUDE_DIR}/${SHADER_NAME}.h"
		  /Vn g_${SHADER_NAME}_bytecode
		"${SHADER_FILE}"
	  RESULT_VARIABLE FXC_RESULT
	  OUTPUT_VARIABLE FXC_OUTPUT
	  ERROR_VARIABLE FXC_ERROR
  )

  if(NOT FXC_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to compile shader: ${SHADER_FILE}\n" "${FXC_OUTPUT}\n" "${FXC_ERROR}")
  endif()
endforeach()

source_group("Shaders" FILES ${SHADER_FILES})
add_custom_target(shader_includes
  SOURCES ${SHADER_FILES}
)
# dummy
add_library(shaders INTERFACE)
target_include_directories(shaders 
  INTERFACE ${SHADER_INCLUDE_DIR}
)
add_dependencies(shaders shader_includes)