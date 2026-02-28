<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
mkdir('a/./b', 0755, true);
if (is_dir('a/b')) {
    rmdir('a/b');
}
if (is_dir('./a')) {
    rmdir('a');
}
echo "OK";
}
