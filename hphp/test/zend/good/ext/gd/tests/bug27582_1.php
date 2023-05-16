<?hh
<<__EntryPoint>> function main(): void {
/* $id$ */
$dest = sys_get_temp_dir().'/'.'bug27582.png';
@unlink($dest);
$im = imagecreatetruecolor(10, 10);
imagealphablending($im, true);
imagesavealpha($im, true);
$bordercolor=imagecolorallocatealpha($im, 0, 0, 0, 2);
$color = imagecolorallocatealpha($im, 0, 0, 0, 1);
imagefilltoborder($im, 5, 5, $bordercolor, $color);
imagepng($im, $dest);

$im2 = imagecreatefrompng($dest);
$col = imagecolorat($im2, 5, 5);
$color = imagecolorsforindex($im2, $col);
echo $color['alpha'];
@unlink($dest);
}
