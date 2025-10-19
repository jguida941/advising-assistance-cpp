# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/advisor_cli_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/advisor_cli_autogen.dir/ParseCache.txt"
  "CMakeFiles/advisor_gui_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/advisor_gui_autogen.dir/ParseCache.txt"
  "CMakeFiles/catalog_core_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/catalog_core_autogen.dir/ParseCache.txt"
  "advisor_cli_autogen"
  "advisor_gui_autogen"
  "catalog_core_autogen"
  )
endif()
