<?hh <<__EntryPoint>> function main(): void {
$file = dirname(__FILE__).'/SplFileObject_fwrite_variation_002.txt';
if(file_exists($file)) {
    unlink($file);
}
$obj = new SplFileObject($file,'w');
$obj->fwrite('test_write',12);
var_dump(file_get_contents($file));
error_reporting(0);
$file = dirname(__FILE__).'/SplFileObject_fwrite_variation_002.txt';
if(file_exists($file)) {
    unlink($file);
}
}
