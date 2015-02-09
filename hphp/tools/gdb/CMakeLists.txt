GET_DIRECTORY_PROPERTY(SOURCE_INCLUDE_DIRS INCLUDE_DIRECTORIES)
GET_DIRECTORY_PROPERTY(HHVM_DEFINITIONS COMPILE_DEFINITIONS)

configure_file(hhvm-gdb.in hhvm-gdb ESCAPE_QUOTES @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hhvm-gdb
  DESTINATION bin
  COMPONENT dev)

install(FILES gdbutils.py hashes.py hhbc.py hhvm.py idx.py
              lookup.py nameof.py pretty.py stack.py unit.py
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/hhvm/gdb"
  COMPONENT dev)
