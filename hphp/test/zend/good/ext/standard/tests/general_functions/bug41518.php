<?hh
<<__EntryPoint>> function main(): void {
$pattern = sys_get_temp_dir().'/bug41518.XXXXXX';
$tmp_dir = HH\Lib\OS\mkdtemp($pattern);
$tmp_file = $tmp_dir."/bug41418.tmp";

touch($tmp_file);
var_dump(file_exists($tmp_file)); //exists
var_dump(file_exists($tmp_file."nosuchfile")); //doesn't exist

@unlink($tmp_file);
@rmdir($tmp_dir);
echo "Done\n";
}
