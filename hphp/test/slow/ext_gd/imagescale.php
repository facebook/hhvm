<?hh


<<__EntryPoint>>
function main_imagescale() :mixed{
$im = imagecreate(256, 256);
$tmpfile = tempnam(sys_get_temp_dir(), 'testimagescale.png');
imagejpeg($im, $tmpfile);
$info = null;
list($width, $height, $type, $attr) = getimagesize($tmpfile, inout $info);
var_dump('width : ' . $width . ' | height : ' . $height);
imagedestroy($im);

$img_src = imagecreatefromjpeg($tmpfile);
$img_dest = imagescale($img_src, 255, 255, IMG_NEAREST_NEIGHBOUR);
imagejpeg($img_dest, $tmpfile);
imagedestroy($img_src);
imagedestroy($img_dest);
list($width, $height, $type, $attr) = getimagesize($tmpfile, inout $info);
var_dump('width : ' . $width . ' | height : ' . $height);

$img_src = imagecreatefromjpeg($tmpfile);
$img_dest = imagescale($img_src, 254, 254, IMG_BILINEAR_FIXED);
imagejpeg($img_dest, $tmpfile);
imagedestroy($img_src);
imagedestroy($img_dest);
list($width, $height, $type, $attr) = getimagesize($tmpfile, inout $info);
var_dump('width : ' . $width . ' | height : ' . $height);

$img_src = imagecreatefromjpeg($tmpfile);
$img_dest = imagescale($img_src, 253, 253, IMG_BICUBIC);
imagejpeg($img_dest, $tmpfile);
imagedestroy($img_src);
imagedestroy($img_dest);
list($width, $height, $type, $attr) = getimagesize($tmpfile, inout $info);
var_dump('width : ' . $width . ' | height : ' . $height);

$img_src = imagecreatefromjpeg($tmpfile);
$img_dest = imagescale($img_src, 252, 252, IMG_BICUBIC_FIXED);
imagejpeg($img_dest, $tmpfile);
imagedestroy($img_src);
imagedestroy($img_dest);
list($width, $height, $type, $attr) = getimagesize($tmpfile, inout $info);
var_dump('width : ' . $width . ' | height : ' . $height);

unlink($tmpfile);
}
