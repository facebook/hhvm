<?hh <<__EntryPoint>> function main(): void {
$im = imagecreate(100, 100);
$black = imagecolorallocate($im, 0, 0, 0);
if (imagefilledpolygon($im, vec[0, 0, 0, 0, 0, 0], -1, $black)) echo "should be false";
imagedestroy($im);
}
