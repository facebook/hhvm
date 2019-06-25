<?hh
<<__EntryPoint>> function main(): void {
$tmp_link = __FILE__.".tmp.link";
$tmp_link2 = __FILE__.".tmp.link2";

$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
symlink($file_path."/there_is_no_such_file", $tmp_link);
rename($tmp_link, $tmp_link2);

clearstatcache();

var_dump(readlink($tmp_link));
var_dump(readlink($tmp_link2));

@unlink($tmp_link);
@unlink($tmp_link2);

echo "Done\n";
}
