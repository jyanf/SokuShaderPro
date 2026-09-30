cmake_minimum_required(VERSION 3.15)
# meta
set(MY_VERSION 0.0.1 CACHE STRING "ShaderPro version (major.minor.patch [.build])")
set(MY_DESCRIPTION "An superset of soku original shading and blending, introducing modern effects." CACHE STRINGS "description")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

project(ShaderPro
  DESCRIPTION ${MY_DESCRIPTION}
  VERSION ${MY_VERSION}
  LANGUAGES C CXX
)
set(PROJECT_DESCRIPTION "More shaders for Soku" CACHE STRINGS "description short ver" FORCE)

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

option(BUILD_EXTRACTOR OFF "build d3dxeffect extractor")
if(${BUILD_EXTRACTOR})
  add_subdirectory("${CMAKE_SOURCE_DIR}/reverse")
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/SokuLib EXCLUDE_FROM_ALL)

# target
add_library(${PROJECT_NAME} SHARED
  ${CMAKE_SOURCE_DIR}/src/export.cpp
  ${CMAKE_SOURCE_DIR}/src/lua_export.cpp
  ${CMAKE_BINARY_DIR}/version.rc
)
add_dependencies(${PROJECT_NAME} SokuLib)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/lua
  ${CMAKE_SOURCE_DIR}/SokuLib/src ${CMAKE_SOURCE_DIR}/SokuLib/directx
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  _CRT_SECURE_NO_WARNINGS
  WINVER=0x0601
  _WIN32_WINNT=0x0601
)
# generator specified
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${PROJECT_NAME} PRIVATE /EHsc /nologo)
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/O2>)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O2>)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)


target_link_libraries(${PROJECT_NAME} PRIVATE
  SokuLib
  Shlwapi
)

# install
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out" CACHE PATH "Install prefix, can be targeted to game modules folder")
# copy dll
install(FILES $<TARGET_FILE:${PROJECT_NAME}> DESTINATION ${PROJECT_NAME})
if (${BUILD_EXTRACTOR})
  message("extra install extractor")
  install(FILES $<TARGET_FILE:extractor> DESTINATION ${PROJECT_NAME})
endif()
# copy pdb
if (MSVC)
  install(
    FILES $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$<TARGET_PDB_FILE:${PROJECT_NAME}>>
    DESTINATION ${PROJECT_NAME}
  )
endif()
# copy assets
install(FILES
  # "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.ini"
  DESTINATION ${PROJECT_NAME}
)