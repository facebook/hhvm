<?hh
<<__EntryPoint>> function main(): void {
$src = sys_get_temp_dir().'/'.'srcbug65701_file.txt';
$dst = sys_get_temp_dir().'/'.'dstbug65701_file.txt';

file_put_contents($src, "Hello World");

copy($src, $dst);
var_dump(filesize($dst));

unlink($dst);
unlink($src);
}
