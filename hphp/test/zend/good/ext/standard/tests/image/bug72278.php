<?hh <<__EntryPoint>> function main(): void {
$filename =  __DIR__ . DIRECTORY_SEPARATOR . 'bug72278.jpg';
$info = null;
var_dump(getimagesize($filename, inout $info));
}
