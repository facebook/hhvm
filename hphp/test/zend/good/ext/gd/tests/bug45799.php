<?hh <<__EntryPoint>> function main() {
$img = imagecreate(500,500);
imagepng($img);
imagedestroy($img);
}
