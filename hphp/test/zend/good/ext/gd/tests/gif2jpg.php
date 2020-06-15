<?hh
<<__EntryPoint>> function main(): void {

echo "GIF to JPEG conversion: ";
echo imagejpeg(
  imagecreatefromgif(__DIR__ . '/conv_test.gif'),
  __SystemLib\hphp_test_tmppath('test_gif.jpeg')
) ? 'ok' : 'failed';
echo "\n";

unlink(__SystemLib\hphp_test_tmppath('test_gif.jpeg'));
}
