<?hh <<__EntryPoint>> function main(): void {
$im = new Imagick ('magick:rose');
$im->convolveImage (varray [1, 'a', 1]);

echo "OK" . PHP_EOL;
}
