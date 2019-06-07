<?hh <<__EntryPoint>> function main() {
$image = imagecreatetruecolor(-1, 30);
$image = imagecreatetruecolor(30, -1);
$image = imagecreatetruecolor(PHP_INT_MAX, 30);
$image = imagecreatetruecolor(30, PHP_INT_MAX);
}
