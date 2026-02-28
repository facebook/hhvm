<?hh
<<__EntryPoint>> function main(): void {
$dirpath = sys_get_temp_dir()."/baz/foo//bar/logs";
mkdir($dirpath, 0777, true);

if (is_dir($dirpath)) {
    echo "Ok.\n";
} else {
    echo "Failed.\n";
}
rmdir(sys_get_temp_dir()."/baz/foo/bar/logs");
rmdir(sys_get_temp_dir()."/baz/foo/bar/");
rmdir(sys_get_temp_dir()."/baz/foo/");
rmdir(sys_get_temp_dir()."/baz/");
}
