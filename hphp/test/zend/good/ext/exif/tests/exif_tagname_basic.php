<?php

/* Prototype  :string exif_tagname ( string $index  )
 * Description: Get the header name for an index
 * Source code: ext/exif/exif.c
*/

echo "*** Testing exif_tagname() : basic functionality ***\n";

var_dump(exif_tagname(0x10E));
var_dump(exif_tagname(0x10F));
var_dump(exif_tagname(0x110));

?>
===Done===