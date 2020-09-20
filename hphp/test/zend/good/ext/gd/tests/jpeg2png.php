<?hh
<<__EntryPoint>> function main(): void {

echo "PNG to JPEG conversion: ";
echo imagejpeg(
  imagecreatefrompng(__DIR__ . '/conv_test.png'),
  __SystemLib\hphp_test_tmppath('test_jpeg.jpeg')
) ? 'ok' : 'failed';
echo "\n";

echo "Generated JPEG to PNG conversion: ";
echo imagepng(
  imagecreatefromjpeg(__SystemLib\hphp_test_tmppath('test_jpeg.jpeg')),
  __SystemLib\hphp_test_tmppath('test_jpng.png')
) ? 'ok' : 'failed';
echo "\n";

echo "JPEG to PNG conversion: ";
echo imagepng(
  imagecreatefromjpeg(__DIR__ . '/conv_test.jpeg'),
  __SystemLib\hphp_test_tmppath('test_png.png')
) ? 'ok' : 'failed';
echo "\n";

echo "Generated PNG to JPEG conversion: ";
echo imagejpeg(
  imagecreatefrompng(__SystemLib\hphp_test_tmppath('test_png.png')),
  __SystemLib\hphp_test_tmppath('test_pjpeg.jpeg')
) ? 'ok' : 'failed';
echo "\n";

unlink(__SystemLib\hphp_test_tmppath('test_jpeg.jpeg'));
unlink(__SystemLib\hphp_test_tmppath('test_jpng.png'));
unlink(__SystemLib\hphp_test_tmppath('test_png.png'));
unlink(__SystemLib\hphp_test_tmppath('test_pjpeg.jpeg'));
}
