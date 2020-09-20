<?hh
<<__EntryPoint>> function main(): void {

echo "GIF to PNG conversion: ";
echo imagepng(
  imagecreatefromgif(__DIR__ . '/conv_test.gif'),
  __SystemLib\hphp_test_tmppath('test_gif.png')
) ? 'ok' : 'failed';
echo "\n";

unlink(__SystemLib\hphp_test_tmppath('test_gif.png'));
}
