<?php
/* Prototype  : string image_type_to_mime_type(int imagetype)
 * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype 
 * Source code: ext/standard/image.c
 */


echo "*** Testing image_type_to_mime_type() : usage variations ***\n";

error_reporting(E_ALL ^ E_NOTICE);

var_dump( image_type_to_mime_type(IMAGETYPE_ICO) );
var_dump( image_type_to_mime_type(IMAGETYPE_SWC) );
?>
===DONE===