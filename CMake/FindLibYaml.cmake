if (LibYaml_LIBRARIES AND LibYaml_INCLUDE_DIRS)
  set (LibYaml_FIND_QUIETLY TRUE)
endif (LibYaml_LIBRARIES AND LibYaml_INCLUDE_DIRS)

find_path (LibYaml_INCLUDE_DIRS NAMES yaml.h)
find_library (LibYaml_LIBRARIES NAMES yaml)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibYaml DEFAULT_MSG
    LibYaml_LIBRARIES
    LibYaml_INCLUDE_DIRS)

mark_as_advanced(LibYaml_INCLUDE_DIRS LibYaml_LIBRARIES)
