<?hh


<<__EntryPoint>>
function main_1793() :mixed{
$filename = 'test/images/test1pix.jpg';
$width = null;
$height = null;
$type = null;
$image = exif_thumbnail($filename, inout $width, inout $height, inout $type);
if ($image!==false) {
  header('Content-type: ' .image_type_to_mime_type($type));
  var_dump($width, $height, $type);
}
 else {
  echo 'No thumbnail available';
}
}
