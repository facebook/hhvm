<?hh <<__EntryPoint>> function main(): void {
$im = new Imagick();
$im->newImage(100, 100, new ImagickPixel("white"));
var_dump($im->valid());
}
