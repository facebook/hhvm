<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);
$resource = tmpfile();

imageistruecolor($resource);
}
