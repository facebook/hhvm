<?hh


// Create a 100*30 image
<<__EntryPoint>>
function main_1790() :mixed{
$im = imagecreate(100, 30);
// White background and blue text
$bg = imagecolorallocate($im, 255, 255, 255);
$textcolor = imagecolorallocate($im, 0, 0, 255);
// Write the string at the top left
imagestring($im, 5, 0, 0, 'Hello world!', $textcolor);
// Output the image
ob_start();
imagepng($im);
$md5 = md5(ob_get_clean());
imagedestroy($im);
echo "md5: $md5\n";
}
