<?hh <<__EntryPoint>> function main(): void {
@mkdir(__SystemLib\hphp_test_tmppath('realpath_basic4/home/test'), 0777, true);
@symlink(
  __SystemLib\hphp_test_tmppath('realpath_basic4/home'),
  __SystemLib\hphp_test_tmppath('realpath_basic4/link1')
);
@symlink(
  __SystemLib\hphp_test_tmppath('realpath_basic4/link1'),
  __SystemLib\hphp_test_tmppath('realpath_basic4/link2')
);
echo "1. " . realpath(__SystemLib\hphp_test_tmppath('realpath_basic4/link2')) . "\n";
echo "2. " . realpath(__SystemLib\hphp_test_tmppath('realpath_basic4/link2/test')) . "\n";

unlink(__SystemLib\hphp_test_tmppath('realpath_basic4/link2'));
unlink(__SystemLib\hphp_test_tmppath('realpath_basic4/link1'));
rmdir(__SystemLib\hphp_test_tmppath('realpath_basic4/home/test'));
rmdir(__SystemLib\hphp_test_tmppath('realpath_basic4/home'));
rmdir(__SystemLib\hphp_test_tmppath('realpath_basic4'));
}
