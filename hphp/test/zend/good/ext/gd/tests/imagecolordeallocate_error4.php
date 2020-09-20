<?hh <<__EntryPoint>> function main(): void {
$image = imagecreate(180, 30);
$white = imagecolorallocate($image, 255, 255, 255);

$totalColors = imagecolorstotal($image);

$result = imagecolordeallocate($image, -1);
var_dump($result);
}
