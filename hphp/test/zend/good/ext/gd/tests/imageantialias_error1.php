<?hh <<__EntryPoint>> function main(): void {
$image = tmpfile();

var_dump(imageantialias($image, true));
}
