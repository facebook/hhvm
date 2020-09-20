<?hh
<<__EntryPoint>> function main(): void {
$src = __SystemLib\hphp_test_tmppath('srcbug65701_file.txt');
$dst = __SystemLib\hphp_test_tmppath('dstbug65701_file.txt');

file_put_contents($src, "Hello World");

copy($src, $dst);
var_dump(filesize($dst));

unlink($dst);
unlink($src);
}
