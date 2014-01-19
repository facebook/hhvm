<?php

$filename = 'test/images/test1pix.jpg';
$image = exif_thumbnail($filename, $width, $height, $type);
if ($image!==false) {
  header('Content-type: ' .image_type_to_mime_type($type));
  var_dump($width, $height, $type);
}
 else {
  echo 'No thumbnail available';
}
