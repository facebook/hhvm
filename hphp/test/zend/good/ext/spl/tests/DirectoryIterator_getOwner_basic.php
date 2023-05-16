<?hh
<<__EntryPoint>> function main(): void {
$dirname = sys_get_temp_dir().'/'.'DirectoryIterator_getOwner_basic';
mkdir($dirname);
$dir = new DirectoryIterator($dirname);
$expected = fileowner($dirname);
$actual = $dir->getOwner();
var_dump($expected == $actual);

rmdir($dirname);
}
