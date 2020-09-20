<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
mkdir('a/./b', 0755, true);
if (is_dir('a/b')) {
    rmdir('a/b');
}
if (is_dir('./a')) {
    rmdir('a');
}
echo "OK";
}
