<?php
/* Prototype  : string image_type_to_mime_type(int imagetype)
 * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype 
 * Source code: ext/standard/image.c
 */

echo "*** Testing image_type_to_mime_type() : usage variations ***\n";

for($imagetype = 0; $imagetype <= IMAGETYPE_COUNT; ++$imagetype) {
  echo "\n-- Iteration $imagetype --\n";
  var_dump(image_type_to_mime_type($imagetype));
}
?>
===DONE===