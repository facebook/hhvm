<?hh
<<__EntryPoint>> function main(): void {
$tmp_empty_file = __SystemLib\hphp_test_tmppath('tmp');
file_put_contents($tmp_empty_file, "");

var_dump(file_get_contents($tmp_empty_file, false, NULL, 0, 10));
unlink($tmp_empty_file);
echo "==DONE==";
}
