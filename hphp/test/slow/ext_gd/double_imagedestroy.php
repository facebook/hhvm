<?hh

<<__EntryPoint>>
function main_double_imagedestroy() {
$im = imagecreate(20,20);
var_dump($im);
var_dump(imagedestroy($im));
var_dump($im);
var_dump(imagedestroy($im));
var_dump($im);
}
