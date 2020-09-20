<?hh <<__EntryPoint>> function main(): void {
$src = imagecreate(100, 100);
imagecolorallocate($src, 255, 255, 255);
$dst = imagescale($src, 200, 200, IMG_BILINEAR_FIXED);
printf("color: %x\n", imagecolorat($dst, 99, 99));
}
