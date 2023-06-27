<?hh
<<__EntryPoint>> function main(): void {
$im = new Imagick();
$im->newPseudoImage(100, 100, "XC:red");
$im->setImageFormat("PNG");

echo $im->width . "x" . $im->height . "\n";
echo $im->format;
}
