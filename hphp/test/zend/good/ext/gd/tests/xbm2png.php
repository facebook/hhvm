<?hh
<<__EntryPoint>> function main(): void {

echo "XBM to PNG conversion: ";
echo imagepng(
  imagecreatefromxbm(__DIR__ . '/conv_test.xbm'),
  sys_get_temp_dir().'/'.'test_xbm.png'
) ? 'ok' : 'failed';
echo "\n";

unlink(sys_get_temp_dir().'/'.'test_xbm.png');
}
