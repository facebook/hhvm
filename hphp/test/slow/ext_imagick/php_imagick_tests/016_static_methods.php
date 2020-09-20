<?hh
<<__EntryPoint>> function main(): void {
echo gettype (Imagick::queryFormats ()) . PHP_EOL;
echo gettype (Imagick::queryFonts ()) . PHP_EOL;
echo 'success';
}
