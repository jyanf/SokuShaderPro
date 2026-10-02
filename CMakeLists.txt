cmake_minimum_required(VERSION 3.21)
# meta
set(MY_VERSION 0.1.0 CACHE STRING "ShaderPro version (major.minor.patch [.build])")
set(MY_DESCRIPTION "An superset of soku original shading and blending, introducing modern effects." CACHE STRING "description")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

project(ShaderPro
  DESCRIPTION ${MY_DESCRIPTION}
  VERSION ${MY_VERSION}
  LANGUAGES C CXX
)
set(PROJECT_DESCRIPTION "More shaders for Soku" CACHE STRING "description short ver" FORCE)

if(NOT WIN32)
  message(FATAL_ERROR "This project can only be build as x86(Win32).")
endif()

# generates version.rc
string(REGEX MATCHALL "[0-9]+" MY_VERSION_PARTS "${MY_VERSION}")
string(TIMESTAMP MY_BUILD_DATE "%Y%m%d")
string(TIMESTAMP MY_BUILD_DATE_SHORT "%m%d")
list(LENGTH MY_VERSION_PARTS _len)
list(APPEND MY_VERSION_PARTS 0 0 0)
list(GET MY_VERSION_PARTS 0 MY_VERSION_MAJOR)
list(GET MY_VERSION_PARTS 1 MY_VERSION_MINOR)
list(GET MY_VERSION_PARTS 2 MY_VERSION_PATCH)
if(_len GREATER 3)
    list(GET MY_VERSION_PARTS 3 MY_VERSION_BUILD)
else()
    set(MY_VERSION_BUILD "${MY_BUILD_DATE_SHORT}")
endif()
configure_file(${CMAKE_SOURCE_DIR}/src/version.rc.in ${CMAKE_BINARY_DIR}/version.rc @ONLY)

# subdirectories
option(BUILD_EXTRACTOR "build d3dxeffect extractor" OFF)
if(${BUILD_EXTRACTOR})
  add_subdirectory("${CMAKE_SOURCE_DIR}/reverse")
endif()

option(BUILD_WITH_SHADERS "compile into binary then header" ON)
if(${BUILD_WITH_SHADERS})
  add_subdirectory("${CMAKE_SOURCE_DIR}/shaders" EXCLUDE_FROM_ALL)
  get_target_property(SHADER_INCLUDE_DIR shaders INTERFACE_INCLUDE_DIRECTORIES)
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/SokuLib" EXCLUDE_FROM_ALL)


# target
file(GLOB_RECURSE GLOB_SOURCE_FILE CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE GLOB_HEADER_FILE CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.hpp" "${CMAKE_SOURCE_DIR}/src/*.h")
source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${GLOB_SOURCE_FILE})
source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "Header Files" FILES ${GLOB_HEADER_FILE})
add_library(${PROJECT_NAME} MODULE
  ${GLOB_SOURCE_FILE} ${GLOB_HEADER_FILE}
  "${CMAKE_BINARY_DIR}/version.rc"
)
if(TARGET extractor)
  add_dependencies(${PROJECT_NAME} extractor)
endif()
if(TARGET shaders)
  add_dependencies(${PROJECT_NAME} shaders)
endif()
add_dependencies(${PROJECT_NAME} SokuLib)

# include
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/lua
  ${CMAKE_SOURCE_DIR}/SokuLib/src ${CMAKE_SOURCE_DIR}/SokuLib/directx
  ${SHADER_INCLUDE_DIR}
)
# link
target_link_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/SokuLib/lib
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  SokuLib
  Shlwapi
  d3dx9
)
# compile
target_compile_definitions(${PROJECT_NAME} PRIVATE
  _CRT_SECURE_NO_WARNINGS
  # XP 
  WINVER=0x0501 _WIN32_WINNT=0x0501
)
# generator specified
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${PROJECT_NAME} PRIVATE /EHsc /nologo $<$<CONFIG:Release>:/O2> $<$<CONFIG:Debug>:/ZI /INCREMENTAL>)
  target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS /Brepro)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-c++11-narrowing -Wno-microsoft-cast)
  endif()
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra $<$<CONFIG:Release>:-O2>)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)


# install
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Install prefix, can be targeted to game folder.")
cmake_path(APPEND CMAKE_INSTALL_PREFIX "modules" "${PROJECT_NAME}" OUTPUT_VARIABLE CMAKE_INSTALL_DIR)
message("Will install to ${CMAKE_INSTALL_DIR}")
# copy dll
install(FILES $<TARGET_FILE:${PROJECT_NAME}> DESTINATION "${CMAKE_INSTALL_DIR}")
if (TARGET extractor)
  message("  extra install extractor")
  install(FILES $<TARGET_FILE:extractor> DESTINATION "${CMAKE_INSTALL_DIR}")
endif()
# copy pdb if debug build
if (MSVC)
  install(
    FILES $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$<TARGET_PDB_FILE:${PROJECT_NAME}>>
    DESTINATION "${CMAKE_INSTALL_DIR}"
  )
endif()
# copy assets
install(FILES
  # "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.ini"
  DESTINATION "${CMAKE_INSTALL_DIR}"
)
install(DIRECTORY
  ${CMAKE_SOURCE_DIR}/shaders
  DESTINATION "${CMAKE_INSTALL_DIR}"
  FILES_MATCHING
    PATTERN "*.fx"
    PATTERN "*.hlsl"
)

# debugging
if (MSVC)
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
  set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
    VS_DEBUGGER_COMMAND "${CMAKE_INSTALL_PREFIX}/th123.exe"
    VS_DEBUGGER_COMMAND_ARGUMENTS ""
)
endif()
add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND tasklist | findstr th123.exe &&
            (taskkill /IM th123.exe /F) ||
            (exit /b 0)
    COMMENT "Kill all of the game instance."
)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND}
        --install "${CMAKE_BINARY_DIR}"
        --config $<CONFIG>
    VERBATIM
    COMMENT "Manually Install after build."
)