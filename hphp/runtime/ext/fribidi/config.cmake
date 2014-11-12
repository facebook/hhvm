find_package(fribidi 0.19.6)
if (FRIBIDI_FOUND)
  find_package(GLIB)
  if (GLIB_FOUND)
    HHVM_EXTENSION(fribidi ext_fribidi.cpp)
    HHVM_ADD_INCLUDES(fribidi ${FRIBIDI_INCLUDE_DIR}
                              ${GLIB_INCLUDE_DIR}
                              ${GLIB_CONFIG_INCLUDE_DIR})
    HHVM_LINK_LIBRARIES(fribidi ${FRIBIDI_LIBRARY})
    HHVM_SYSTEMLIB(fribidi ext_fribidi.php)
  endif()
endif()
