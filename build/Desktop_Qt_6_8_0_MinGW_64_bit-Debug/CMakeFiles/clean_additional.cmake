# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\TimeEditor_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\TimeEditor_autogen.dir\\ParseCache.txt"
  "TimeEditor_autogen"
  )
endif()