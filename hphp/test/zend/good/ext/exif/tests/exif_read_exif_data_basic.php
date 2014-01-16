<?php

/* Prototype  : array read_exif_data  ( string $filename  [, string $sections  [, bool $arrays  [, bool $thumbnail  ]]] )
 * Description: Alias of exif_read_data()
 * Source code: ext/exif/exif.c
*/
echo "*** Testing read_exif_data() : basic functionality ***\n";

print_r(read_exif_data(dirname(__FILE__).'/test2.jpg'));
?>
===Done===