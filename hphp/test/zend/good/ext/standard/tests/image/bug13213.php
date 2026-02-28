<?hh <<__EntryPoint>> function main(): void {
$info = null;
var_dump(getimagesize(dirname(__FILE__).'/bug13213.jpg', inout $info));
}
