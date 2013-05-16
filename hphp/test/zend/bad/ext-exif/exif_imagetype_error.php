<?php

/* Prototype  : int exif_imagetype  ( string $filename  )
 * Description: Determine the type of an image
 * Source code: ext/exif/exif.c
*/

echo "*** Testing exif_imagetype() : error conditions ***\n";

echo "\n-- Testing exif_imagetype() function with no arguments --\n";
var_dump( exif_imagetype() );

echo "\n-- Testing exif_imagetype() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( exif_imagetype(dirname(__FILE__).'/test2.jpg', $extra_arg) );

echo "\n-- Testing exif_imagetype() function with an unknown file  --\n";
var_dump( exif_imagetype(dirname(__FILE__).'/foo.jpg') );


?>
===Done===