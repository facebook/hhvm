<?hh
<<__EntryPoint>> function main(): void {
$tmp_file = sys_get_temp_dir().'/'.'rename_variation6.php.tmp';
$tmp_link = sys_get_temp_dir().'/'.'rename_variation6.php.tmp.link';
$tmp_link2 = sys_get_temp_dir().'/'.'rename_variation6.php.tmp.link2';

touch($tmp_file);
symlink($tmp_file, $tmp_link);
rename($tmp_link, $tmp_link2);

clearstatcache();

var_dump(readlink($tmp_link));
var_dump(readlink($tmp_link2));
var_dump(file_exists($tmp_file));

@unlink($tmp_link);
@unlink($tmp_link2);
@unlink($tmp_file);

echo "Done\n";
}
