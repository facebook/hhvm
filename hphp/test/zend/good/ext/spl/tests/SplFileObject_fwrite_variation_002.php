<?hh
<<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('SplFileObject_fwrite_variation_002.txt');
$obj = new SplFileObject($file,'w');
$obj->fwrite('test_write',12);
var_dump(file_get_contents($file));

unlink($file);
}
