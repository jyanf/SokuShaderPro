

add_library(extractor SHARED
  "main.cpp"
)

target_include_directories(extractor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/SokuLib/src
  ${CMAKE_SOURCE_DIR}/SokuLib/directx
)

target_link_directories(extractor PUBLIC
  ${CMAKE_SOURCE_DIR}/SokuLib/lib
)

target_link_libraries(extractor PUBLIC
  SokuLib
  Shlwapi
  d3dx9
)