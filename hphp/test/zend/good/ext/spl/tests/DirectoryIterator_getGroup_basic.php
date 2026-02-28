<?hh
<<__EntryPoint>> function main(): void {
$dirname = sys_get_temp_dir().'/'.'DirectoryIterator_getGroup_basic';
mkdir($dirname);
$dir = new DirectoryIterator($dirname);
$expected = filegroup($dirname);
$actual = $dir->getGroup();
var_dump($expected == $actual);

rmdir($dirname);
}
