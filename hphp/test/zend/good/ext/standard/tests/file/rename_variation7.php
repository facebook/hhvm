<?hh
<<__EntryPoint>> function main(): void {
$tmp_link = sys_get_temp_dir().'/'.'tmp.link';
$tmp_link2 = sys_get_temp_dir().'/'.'tmp.link2';

symlink(sys_get_temp_dir().'/there_is_no_such_file', $tmp_link);
rename($tmp_link, $tmp_link2);

clearstatcache();

var_dump(readlink($tmp_link));
var_dump(readlink($tmp_link2));

unlink($tmp_link2);

echo "Done\n";
}
