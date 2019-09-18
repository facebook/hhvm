<?hh
require_once 'func.inc';
<<__EntryPoint>> function main(): void {
$im = imagecreatetruecolor(10, 10);
imagefilledrectangle($im, 0, 0, 9, 9, imagecolorallocate($im, 255, 255, 255));
imageantialias($im, true);
imageline($im, 0, 0, 10, 10, imagecolorallocate($im, 0, 0, 0));

test_image_equals_file(__DIR__ . DIRECTORY_SEPARATOR . 'bug72482_2.png', $im);
}
