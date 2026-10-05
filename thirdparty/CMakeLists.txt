# SokuLib
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/SokuLib" EXCLUDE_FROM_ALL)
# fix d3dx9.lib not linked
target_link_directories(SokuLib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/SokuLib/lib
)
target_link_libraries(SokuLib PUBLIC d3dx9)
target_include_directories(SokuLib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/SokuLib/src
  ${CMAKE_CURRENT_SOURCE_DIR}/SokuLib/directx
)

# lua
# find_package(lua QUIET CONFIG REQUIRED)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lua" EXCLUDE_FROM_ALL)

# Luabridge
# add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge" EXCLUDE_FROM_ALL)
add_library(LuaBridge INTERFACE)
target_link_libraries(LuaBridge INTERFACE lua::lua)
target_include_directories(LuaBridge INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/LuaBridge/Distribution"
)
