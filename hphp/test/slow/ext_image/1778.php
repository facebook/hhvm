<?hh


<<__EntryPoint>>
function main_1778() :mixed{
$im = imagecreatetruecolor(120, 20);
$text_color = imagecolorallocate($im, 233, 14, 91);
imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);
ob_start();
imagepng($im);
$md5 = md5(ob_get_clean());
imagedestroy($im);
echo "md5: $md5\n";
}
