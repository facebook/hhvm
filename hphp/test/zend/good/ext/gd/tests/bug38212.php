<?hh <<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('bug38212.gd2');
$im1 = imagecreatetruecolor(10,100);
imagefill($im1, 0,0, 0xffffff);
imagegd2($im1, $file);
$im = imagecreatefromgd2part($file, 0,0, -25,10);
unlink($file);
}
