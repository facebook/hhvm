<?hh


<<__EntryPoint>>
function main_ico() :mixed{
var_dump(IMAGETYPE_ICO);
var_dump(image_type_to_mime_type(IMAGETYPE_ICO));
var_dump(image_type_to_extension(IMAGETYPE_ICO));
var_dump(image_type_to_extension(IMAGETYPE_ICO, false));
$info = null;
var_dump(getimagesize(__DIR__ . '/images/php.ico', inout $info));
var_dump(getimagesize(__DIR__ . '/images/php_file.ico', inout $info));
}
