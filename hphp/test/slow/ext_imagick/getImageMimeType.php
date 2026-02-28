<?hh


<<__EntryPoint>>
function main_get_image_mime_type() :mixed{
$im = new Imagick;
$im->readImage(__DIR__ . '/facebook.png');
var_dump($im->getImageMimeType());
}
