<?hh
<<__EntryPoint>> function main(): void {
$img = __DIR__ . '/test.gif';
$info = null;
$i1 = getimagesize($img, inout $info);

$data = file_get_contents($img);

$i2 = getimagesizefromstring($data, inout $info);

var_dump($i1);
var_dump($i2);
}
