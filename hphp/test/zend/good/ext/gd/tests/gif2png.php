<?hh
<<__EntryPoint>> function main(): void {

echo "GIF to PNG conversion: ";
echo imagepng(
  imagecreatefromgif(__DIR__ . '/conv_test.gif'),
  sys_get_temp_dir().'/'.'test_gif.png'
) ? 'ok' : 'failed';
echo "\n";

unlink(sys_get_temp_dir().'/'.'test_gif.png');
}
