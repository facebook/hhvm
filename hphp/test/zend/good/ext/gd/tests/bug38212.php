<?hh <<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'bug38212.gd2';
$im1 = imagecreatetruecolor(10,100);
imagefill($im1, 0,0, 0xffffff);
imagegd2($im1, $file);
$im = imagecreatefromgd2part($file, 0,0, -25,10);
unlink($file);
}
