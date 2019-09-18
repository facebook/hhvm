<?hh
require_once 'func.inc';
<<__EntryPoint>> function main(): void {
$src = imagecreatetruecolor(100, 100);
imagefilledrectangle($src, 0,0, 99,99, 0xFFFFFF);
imageellipse($src, 49,49, 40,40, 0x000000);

imagesetinterpolation($src, IMG_NEAREST_NEIGHBOUR);
imagescale($src, 200, 200, IMG_BILINEAR_FIXED);
$dst = imagerotate($src, 60.0, 0xFFFFFF);

test_image_equals_file(__DIR__ . DIRECTORY_SEPARATOR . 'bug73272.png', $dst);
}
