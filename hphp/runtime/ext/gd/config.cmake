if ((NOT DEFINED EXT_GD) OR (EXT_GD STREQUAL "ON"))
  HHVM_EXTENSION(gd ext_gd.cpp
    libgd/gd_arc.cpp
    libgd/gd_arc_f_buggy.cpp
    libgd/gdcache.cpp
    libgd/gd_color.cpp
    libgd/gd.cpp
    libgd/gd_crop.cpp
    libgd/gd_filter.cpp
    libgd/gdfontg.cpp
    libgd/gdfontl.cpp
    libgd/gdfontmb.cpp
    libgd/gdfonts.cpp
    libgd/gdfontt.cpp
    libgd/gdft.cpp
    libgd/gd_gd2.cpp
    libgd/gd_gd.cpp
    libgd/gd_gif_in.cpp
    libgd/gd_gif_out.cpp
    libgd/gdhelpers.cpp
    libgd/gd_interpolation.cpp
    libgd/gd_io.cpp
    libgd/gd_io_dp.cpp
    libgd/gd_io_file.cpp
    libgd/gd_io_ss.cpp
    libgd/gd_jpeg.cpp
    libgd/gdkanji.cpp
    libgd/gd_matrix.cpp
    libgd/gd_pixelate.cpp
    libgd/gd_png.cpp
    libgd/gd_rotate.cpp
    libgd/gd_security.cpp
    libgd/gd_ss.cpp
    libgd/gdtables.cpp
    libgd/gd_topal.cpp
    libgd/gd_transform.cpp
    libgd/gd_wbmp.cpp
    libgd/gd_webp.cpp
    libgd/wbmp.cpp
    libgd/webpimg.cpp
    libgd/xbm.cpp)
  HHVM_SYSTEMLIB(gd ext_gd.php ext_exif.php)

  # GD checks
  HHVM_DEFINE(gd -DPNG_SKIP_SETJMP_CHECK)
  find_package(LibJpeg REQUIRED)
  if (LIBJPEG_INCLUDE_DIRS AND LIBJPEG_LIBRARIES)
    HHVM_LINK_LIBRARIES(gd ${LIBJPEG_LIBRARIES})
    HHVM_ADD_INCLUDES(gd ${LIBJPEG_INCLUDE_DIRS})
    HHVM_DEFINE(gd "-DHAVE_GD_JPG")
  endif()
  find_package(LibPng REQUIRED)
  if (LIBPNG_INCLUDE_DIRS AND LIBPNG_LIBRARIES)
    HHVM_LINK_LIBRARIES(gd ${LIBPNG_LIBRARIES})
    HHVM_ADD_INCLUDES(gd ${LIBPNG_INCLUDE_DIRS})
    HHVM_DEFINE(gd "-DHAVE_GD_PNG")
  endif()
  find_package(LibVpx)
  if (LIBVPX_INCLUDE_DIRS AND LIBVPX_LIBRARIES)
    HHVM_LINK_LIBRARIES(gd ${LIBVPX_LIBRARIES})
    HHVM_ADD_INCLUDES(gd ${LIBVPX_INCLUDE_DIRS})
    HHVM_DEFINE(gd "-DHAVE_LIBVPX")
  endif()
  find_package(Freetype)
  if (FREETYPE_INCLUDE_DIRS AND FREETYPE_LIBRARIES)
    HHVM_LINK_LIBRARIES(gd ${FREETYPE_LIBRARIES})
    HHVM_ADD_INCLUDES(gd ${FREETYPE_INCLUDE_DIRS})
    HHVM_DEFINE(gd "-DHAVE_LIBFREETYPE -DHAVE_GD_FREETYPE -DENABLE_GD_TTF")
  endif()

endif()
