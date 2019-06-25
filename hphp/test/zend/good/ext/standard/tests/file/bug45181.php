<?hh <<__EntryPoint>> function main(): void {
mkdir("bug45181_x");
var_dump(is_dir("bug45181_x"));
$origdir = getcwd();
chdir("bug45181_x");
var_dump(is_dir("bug45181_x"));
error_reporting(0);
rmdir($origdir . "/bug45181_x");
}
