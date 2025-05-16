find_path(BPF_INCLUDE_DIR bpf/bpf.h)
find_library(BPF_LIBRARY NAMES bpf)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BPF DEFAULT_MSG BPF_LIBRARY BPF_INCLUDE_DIR)

if(BPF_FOUND)
  set(BPF_LIBRARIES ${BPF_LIBRARY})
  set(BPF_INCLUDE_DIRS ${BPF_INCLUDE_DIR})
else()
  set(BPF_LIBRARIES)
  set(BPF_INCLUDE_DIRS)
endif()

mark_as_advanced(BPF_LIBRARIES BPF_INCLUDE_DIRS)
