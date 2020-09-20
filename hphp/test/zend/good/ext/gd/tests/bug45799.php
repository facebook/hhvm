<?hh <<__EntryPoint>> function main(): void {
$img = imagecreate(500,500);
imagepng($img);
imagedestroy($img);
}
