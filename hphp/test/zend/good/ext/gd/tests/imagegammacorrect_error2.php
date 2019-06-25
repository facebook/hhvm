<?hh <<__EntryPoint>> function main(): void {
$image = tmpfile();
$gamma = imagegammacorrect($image, 1.0, 5.0);
}
