<?hh <<__EntryPoint>> function main(): void {
$_type = 42;
$height = $_type;
$width = $height;
$s = exif_thumbnail(__DIR__."/bug77540.jpg", inout $width, inout $height, inout $_type);
echo "Width ".$width."\n";
echo "Height ".$height."\n";
}
