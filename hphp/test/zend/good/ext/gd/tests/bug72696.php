<?hh <<__EntryPoint>> function main(): void {
$im = imagecreatetruecolor(10, 10);
imagefilltoborder($im, 0, 0, 1, -2);
}
