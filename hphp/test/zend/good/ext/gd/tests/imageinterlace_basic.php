<?hh
<<__EntryPoint>> function main() {
$image = imagecreatetruecolor(100, 100);
var_dump(imageinterlace($image));
}
