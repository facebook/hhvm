<?hh


<<__EntryPoint>>
function main_issue_read_image_blob_filename_optional2213() :mixed{
$blob = file_get_contents(__DIR__.'/facebook.png');

$im = new Imagick();

var_dump($im->readImageBlob($blob));
}
