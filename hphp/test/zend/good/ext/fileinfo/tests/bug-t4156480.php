<?hh <<__EntryPoint>> function main(): void {
$test_file = __SystemLib\hphp_test_tmppath('test_file_t4156480');
file_put_contents($test_file, "\x45\x52\x00\x00\x00\x00\x00\x00");
$finfo = finfo_open();
$file = finfo_file($finfo, $test_file);
var_dump($file);
unlink($test_file);
}
