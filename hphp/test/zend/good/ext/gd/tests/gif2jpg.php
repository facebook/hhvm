<?hh
<<__EntryPoint>> function main(): void {

echo "GIF to JPEG conversion: ";
echo imagejpeg(
  imagecreatefromgif(__DIR__ . '/conv_test.gif'),
  sys_get_temp_dir().'/'.'test_gif.jpeg'
) ? 'ok' : 'failed';
echo "\n";

unlink(sys_get_temp_dir().'/'.'test_gif.jpeg');
}
