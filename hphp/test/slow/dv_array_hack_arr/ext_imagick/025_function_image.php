<?hh <<__EntryPoint>> function main(): void {
$im = new Imagick ('magick:rose');
$im->convolveimage (varray [1, 'a', 1]);

echo "OK" . PHP_EOL;
}
