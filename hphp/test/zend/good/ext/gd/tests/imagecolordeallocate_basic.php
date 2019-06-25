<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);

$white = imagecolorallocate($image, 255, 255, 255);
$result = imagecolordeallocate($image, $white);

var_dump($result);
}
