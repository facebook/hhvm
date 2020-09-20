<?hh <<__EntryPoint>> function main(): void {
$info = null;
var_dump(getimagesize(dirname(__FILE__) . "/test13pix.swf", inout $info));
}
