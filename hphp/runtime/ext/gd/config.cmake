HHVM_DEFINE_EXTENSION("gd"
  IS_ENABLED EXT_GD
  SOURCES
    ext_gd.cpp
    libgd/gd_arc.cpp
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
    libgd/wbmp.cpp
    libgd/xbm.cpp
  HEADERS
    ext_gd.h
    libgd/gd.h
    libgd/gd_intern.h
    libgd/gd_io.h
    libgd/gdcache.h
    libgd/gdfontg.h
    libgd/gdfontl.h
    libgd/gdfontmb.h
    libgd/gdfonts.h
    libgd/gdfontt.h
    libgd/jisx0208.h
    libgd/wbmp.h
  SYSTEMLIB
    ext_exif.php
    ext_gd.php
  DEPENDS
    libFreetype OPTIONAL
    libJpeg OPTIONAL
    libHeif
    libIConv
    libPng OPTIONAL
    libVpx OPTIONAL
)
