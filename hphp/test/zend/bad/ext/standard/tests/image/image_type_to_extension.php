<?php
    $constants = array(
        "IMAGETYPE_GIF"      => IMAGETYPE_GIF,
        "IMAGETYPE_JPEG"     => IMAGETYPE_JPEG,
        "IMAGETYPE_PNG"      => IMAGETYPE_PNG,
        "IMAGETYPE_SWF"      => IMAGETYPE_SWF,
        "IMAGETYPE_PSD"      => IMAGETYPE_PSD,
        "IMAGETYPE_BMP"      => IMAGETYPE_BMP,
        "IMAGETYPE_TIFF_II"  => IMAGETYPE_TIFF_II,
        "IMAGETYPE_TIFF_MM"  => IMAGETYPE_TIFF_MM,
        "IMAGETYPE_JPC"      => IMAGETYPE_JPC,
        "IMAGETYPE_JP2"      => IMAGETYPE_JP2,
        "IMAGETYPE_JPX"      => IMAGETYPE_JPX,
        "IMAGETYPE_JB2"      => IMAGETYPE_JB2,
        "IMAGETYPE_IFF"      => IMAGETYPE_IFF,
        "IMAGETYPE_WBMP"     => IMAGETYPE_WBMP,
        "IMAGETYPE_JPEG2000" => IMAGETYPE_JPEG2000,
        "IMAGETYPE_XBM"      => IMAGETYPE_XBM
    );
    foreach($constants as $name => $constant) {
        printf("Constant: %s\n\tWith dot: %s\n\tWithout dot: %s\n", $name, image_type_to_extension($constant), image_type_to_extension($constant, false));
    }

	var_dump(image_type_to_extension(-1, array()));
	var_dump(image_type_to_extension(new stdclass));
	var_dump(image_type_to_extension(1000000, NULL));
	var_dump(image_type_to_extension());
	var_dump(image_type_to_extension(0));
	var_dump(image_type_to_extension(0, 0, 0));
?>
Done