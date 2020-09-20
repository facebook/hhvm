<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
$test_dirname = 'testdir';
mkdir($test_dirname);

$filepath = __SystemLib\hphp_test_tmppath('file_variation5.php.tmp');
$filename = basename($filepath);
$fd = fopen($filepath, "w+");
fwrite($fd, "Line 1\nLine 2\nLine 3");
fclose($fd);

echo "file() on a path containing .. and .\n";
var_dump(file("./$test_dirname/../$filename"));

echo "\nfile() on a path containing .. with invalid directories\n";
var_dump(file("./$test_dirname/bad_dir/../../$filename"));
 
echo "\nfile() on a linked file\n";
$linkname = "somelink";
var_dump(symlink($filepath, $linkname));
var_dump(file($linkname));
var_dump(unlink($linkname));

echo "\nfile() on a relative path from a different working directory\n";
chdir($test_dirname);
var_dump(file("../$filename"));

rmdir(__SystemLib\hphp_test_tmppath($test_dirname));
unlink($filepath);
}
