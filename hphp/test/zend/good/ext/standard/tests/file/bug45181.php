<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
mkdir("bug45181_x");
var_dump(is_dir("bug45181_x"));
chdir("bug45181_x");
var_dump(is_dir("bug45181_x"));

rmdir(__SystemLib\hphp_test_tmppath('bug45181_x'));
}
