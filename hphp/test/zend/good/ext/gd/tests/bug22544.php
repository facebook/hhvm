<?hh <<__EntryPoint>> function main(): void {
$dest = sys_get_temp_dir().'/'.'bug22544.png';
@unlink($dest);
$image = imagecreatetruecolor(640, 100);
$transparent = imagecolorallocate($image, 0, 0, 0);
$red = imagecolorallocate($image, 255, 50, 50);
imagecolortransparent($image, $transparent);
imagefilledrectangle($image, 0, 0, 640-1, 100-1, $transparent);
imagepng($image, $dest);
echo md5_file($dest) . "\n";
@unlink($dest);
}
