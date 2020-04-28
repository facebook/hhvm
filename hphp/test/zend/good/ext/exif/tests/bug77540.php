<?hh <<__EntryPoint>> function main(): void {
$width = $height = $_type = 42;
$s = exif_thumbnail(__DIR__."/bug77540.jpg", inout $width, inout $height, inout $_type);
echo "Width ".$width."\n";
echo "Height ".$height."\n";
}
