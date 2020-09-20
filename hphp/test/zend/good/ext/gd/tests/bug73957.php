<?hh <<__EntryPoint>> function main(): void {
$im = imagecreate(8, 8);
$im = imagescale($im, 0x100000001, 1);
var_dump($im);
if ($im) { // which is not supposed to happen
    var_dump(imagesx($im));
}
}
