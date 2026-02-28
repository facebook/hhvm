<?hh
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'SplFileObject_fwrite_variation_001.txt';
$obj = new SplFileObject($file,'w');
$obj->fwrite('test_write',4);
var_dump(file_get_contents($file));

unlink($file);
}
