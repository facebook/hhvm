<?hh <<__EntryPoint>> function main() {
$image = tmpfile();

var_dump(imageantialias($image, true));
}
