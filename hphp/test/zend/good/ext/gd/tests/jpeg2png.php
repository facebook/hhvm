<?hh
<<__EntryPoint>> function main(): void {

echo "PNG to JPEG conversion: ";
echo imagejpeg(
  imagecreatefrompng(__DIR__ . '/conv_test.png'),
  sys_get_temp_dir().'/'.'test_jpeg.jpeg'
) ? 'ok' : 'failed';
echo "\n";

echo "Generated JPEG to PNG conversion: ";
echo imagepng(
  imagecreatefromjpeg(sys_get_temp_dir().'/'.'test_jpeg.jpeg'),
  sys_get_temp_dir().'/'.'test_jpng.png'
) ? 'ok' : 'failed';
echo "\n";

echo "JPEG to PNG conversion: ";
echo imagepng(
  imagecreatefromjpeg(__DIR__ . '/conv_test.jpeg'),
  sys_get_temp_dir().'/'.'test_png.png'
) ? 'ok' : 'failed';
echo "\n";

echo "Generated PNG to JPEG conversion: ";
echo imagejpeg(
  imagecreatefrompng(sys_get_temp_dir().'/'.'test_png.png'),
  sys_get_temp_dir().'/'.'test_pjpeg.jpeg'
) ? 'ok' : 'failed';
echo "\n";

unlink(sys_get_temp_dir().'/'.'test_jpeg.jpeg');
unlink(sys_get_temp_dir().'/'.'test_jpng.png');
unlink(sys_get_temp_dir().'/'.'test_png.png');
unlink(sys_get_temp_dir().'/'.'test_pjpeg.jpeg');
}
