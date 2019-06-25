<?hh <<__EntryPoint>> function main(): void {
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
@mkdir("$file_path/realpath_basic4/home/test", 0777, true);
@symlink("$file_path/realpath_basic4/home", "$file_path/realpath_basic4/link1");
@symlink("$file_path/realpath_basic4/link1", "$file_path/realpath_basic4/link2");
echo "1. " . realpath("$file_path/realpath_basic4/link2") . "\n";
echo "2. " . realpath("$file_path/realpath_basic4/link2/test") . "\n";
error_reporting(0);
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
unlink("$file_path/realpath_basic4/link2");
unlink("$file_path/realpath_basic4/link1");
rmdir("$file_path/realpath_basic4/home/test");
rmdir("$file_path/realpath_basic4/home");
rmdir("$file_path/realpath_basic4");
}
