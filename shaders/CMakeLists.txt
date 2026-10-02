find_program(FXC_EXECUTABLE NAMES fxc.exe PATHS "$ENV{DXSDK_DIR}Utilities/bin" PATH_SUFFIXES x64 x86)
if(NOT FXC_EXECUTABLE)
  message(FATAL_ERROR "fxc.exe was not found. " "Please install the DirectX SDK / Windows SDK or add fxc.exe to PATH." )
endif()
message(STATUS "Using fxc: ${FXC_EXECUTABLE}")
message("fxc.exe from DXSDK_Jun10 is recommended.")
set(SHADER_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${SHADER_INCLUDE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/asm" "${CMAKE_CURRENT_BINARY_DIR}/bin")

file(GLOB_RECURSE SHADER_FILES
  CONFIGURE_DEPENDS 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.fx"
  # "${CMAKE_CURRENT_SOURCE_DIR}/*.hlsl"
)
if(NOT SHADER_FILES)
  message(STATUS "No shader sources found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
set(COMPILED_SHADER_OUTS "")
set(COMPILED_SHADER_HEADERS "")
foreach(SHADER_FILE IN LISTS SHADER_FILES)
  get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
  set(HEADER_FILE "${SHADER_INCLUDE_DIR}/${SHADER_NAME}.h")
  
  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" SHADER_VAR "${SHADER_NAME}")
  if(SHADER_VAR MATCHES "^[0-9]")
    set(SHADER_VAR "_${SHADER_VAR}")
  endif()
  set(OUT_FXO "${CMAKE_CURRENT_BINARY_DIR}/bin/${SHADER_NAME}.fxo")
  set(OUT_FXC "${CMAKE_CURRENT_BINARY_DIR}/asm/${SHADER_NAME}.fxc")
  set(OUT_FXH "${SHADER_INCLUDE_DIR}/fx_${SHADER_NAME}.h")
  # comiple to binary
  execute_process(
	COMMAND "${FXC_EXECUTABLE}"
	  /T fx_2_0 
	  /O3
	  # /Fo "${OUT_FXO}"
	  # /Fc "${OUT_FXC}"
	  /Fh "${OUT_FXH}" /Vn fx_${SHADER_VAR}_bytecode
	  "${SHADER_FILE}"
	RESULT_VARIABLE FXC_RESULT
	OUTPUT_VARIABLE FXC_OUTPUT
	ERROR_VARIABLE FXC_ERROR
  )
  if(NOT FXC_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to compile shader at configure time: ${SHADER_FILE}\n" "${FXC_OUTPUT}\n" "${FXC_ERROR}")
  endif()
  add_custom_command(VERBATIM
    OUTPUT "${OUT_FXH}" "${OUT_FXO}" "${OUT_FXC}"
	COMMAND "${FXC_EXECUTABLE}"
	  /T fx_2_0 
	  /O3
	  /Fo "${OUT_FXO}"
	  /Fc "${OUT_FXC}"
	  /Fh "${OUT_FXH}" /Vn fx_${SHADER_VAR}_bytecode
	  "${SHADER_FILE}"
	DEPENDS "${SHADER_FILE}"
	COMMENT "Compiling full shader files."
  )
  list(APPEND COMPILED_SHADER_OUTS "${OUT_FXO}" "${OUT_FXC}" "${OUT_FXH}")
  list(APPEND COMPILED_SHADER_HEADERS "${OUT_FXH}")
endforeach()

source_group("Shaders" FILES ${SHADER_FILES})
source_group("Compiled Headers" FILES ${COMPILED_SHADER_HEADERS})
if(COMPILED_SHADER_OUTS)
  add_custom_target(shader_includes
    SOURCES
	  ${SHADER_FILES}
	  ${COMPILED_SHADER_HEADERS}
    DEPENDS ${COMPILED_SHADER_OUTS}
  )
else()
  add_custom_target(shader_includes SOURCES ${SHADER_FILES})
endif()

# dummy
add_library(shaders INTERFACE)
target_include_directories(shaders INTERFACE ${SHADER_INCLUDE_DIR})
add_dependencies(shaders shader_includes)