<?hh

<<__EntryPoint>>
function main(): void {
$file = sys_get_temp_dir().'/'.'__tmp_rose.jpg';

$imagick = new Imagick('magick:rose');
$imagick->setImageFormat('jpg');
$imagick->writeImage($file);

$imagick->clear();

$handle = fopen($file, 'rb');
$imagick->readImageFile($handle);

unlink($file);

echo 'success';
}
