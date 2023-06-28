<?hh


// Create a 55x30 image
<<__EntryPoint>>
function main_1779() :mixed{
$im = imagecreatetruecolor(55, 30);
$red = imagecolorallocate($im, 255, 0, 0);
$black = imagecolorallocate($im, 0, 0, 0);
// Make the background transparent
imagecolortransparent($im, $black);
// Draw a red rectangle
imagefilledrectangle($im, 4, 4, 50, 25, $red);
// Save the image
ob_start();
imagepng($im);
$md5 = md5(ob_get_clean());
imagedestroy($im);
echo "md5: $md5\n";
}
