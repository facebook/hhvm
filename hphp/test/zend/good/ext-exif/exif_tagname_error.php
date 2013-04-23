<?php

/* Prototype  :string exif_tagname ( string $index  )
 * Description: Get the header name for an index
 * Source code: ext/exif/exif.c
*/

echo "*** Testing exif_tagname() : error conditions ***\n";

echo "\n-- Testing exif_tagname() function with no arguments --\n";
var_dump( exif_tagname() );

echo "\n-- Testing exif_tagname() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( exif_tagname(0x10E, $extra_arg) );

?>
===Done===