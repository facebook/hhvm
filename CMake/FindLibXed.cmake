find_path(LibXed_INCLUDE_DIR xed-interface.h)
find_library(LibXed_LIBRARY NAMES xed)

mark_as_advanced(LibXed_INCLUDE_DIR LibXed_LIBRARY)
