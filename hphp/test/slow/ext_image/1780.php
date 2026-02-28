<?hh


<<__EntryPoint>>
function main_1780() :mixed{
$image = imagecreatefromgif(__DIR__.'/images/php.gif');
$emboss = vec[vec[2, 0, 0], vec[0, -1, 0], vec[0, 0, -1]];
imageconvolution($image, $emboss, 1.0, 127.0);
header('Content-Type: image/png');
ob_start();
imagepng($image, '', 9);
var_dump(substr(ob_get_clean(),0,10));
}
