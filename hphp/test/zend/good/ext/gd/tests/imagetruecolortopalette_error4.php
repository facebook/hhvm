<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(50, 50);

imagetruecolortopalette($image, true, 0);
imagetruecolortopalette($image, true, -1);
}
