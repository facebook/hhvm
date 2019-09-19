<?hh
require_once 'func.inc';
<<__EntryPoint>> function main(): void {
$im = imagecreatetruecolor(32768, 1);
$black = imagecolorallocate($im, 0, 0, 0);
imageantialias($im, true);

imageline($im, 0,0, 32767,0, $black);

test_image_equals_file(__DIR__ . DIRECTORY_SEPARATOR . 'bug73213.png', $im);
}
