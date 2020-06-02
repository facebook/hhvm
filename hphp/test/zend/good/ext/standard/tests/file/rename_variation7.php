<?hh
<<__EntryPoint>> function main(): void {
$tmp_link = __SystemLib\hphp_test_tmppath('tmp.link');
$tmp_link2 = __SystemLib\hphp_test_tmppath('tmp.link2');

symlink(__SystemLib\hphp_test_tmppath('there_is_no_such_file'), $tmp_link);
rename($tmp_link, $tmp_link2);

clearstatcache();

var_dump(readlink($tmp_link));
var_dump(readlink($tmp_link2));

unlink($tmp_link2);

echo "Done\n";
}
