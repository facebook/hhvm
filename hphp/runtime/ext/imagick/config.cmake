HHVM_EXT_OPTION(IMAGICK LibMagickWand)

if (LIBMAGICKWAND_INCLUDE_DIRS AND
    LIBMAGICKWAND_LIBRARIES AND
    LIBMAGICKCORE_LIBRARIES)
	HHVM_EXTENSION(imagick constants.cpp ext_imagick.cpp 
                               imagick.cpp imagickdraw.cpp
                               imagickpixel.cpp
                               imagickpixeliterator.cpp)
	HHVM_SYSTEMLIB(imagick ext_imagick.php)
	HHVM_ADD_INCLUDES(imagick ${LIBMAGICKWAND_INCLUDE_DIRS})
	HHVM_LINK_LIBRARIES(imagick ${LIBMAGICKWAND_LIBRARIES}
                                    ${LIBMAGICKCORE_LIBRARIES})
endif()

