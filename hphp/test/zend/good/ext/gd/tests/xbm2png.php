<?hh
<<__EntryPoint>> function main(): void {

echo "XBM to PNG conversion: ";
echo imagepng(
  imagecreatefromxbm(__DIR__ . '/conv_test.xbm'),
  __SystemLib\hphp_test_tmppath('test_xbm.png')
) ? 'ok' : 'failed';
echo "\n";

unlink(__SystemLib\hphp_test_tmppath('test_xbm.png'));
}
