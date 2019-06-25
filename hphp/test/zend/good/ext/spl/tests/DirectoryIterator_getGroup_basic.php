<?hh <<__EntryPoint>> function main(): void {
$dirname = 'DirectoryIterator_getGroup_basic';
mkdir($dirname);
$dir = new DirectoryIterator($dirname);
$expected = filegroup($dirname);
$actual = $dir->getGroup();
var_dump($expected == $actual);
error_reporting(0);
$dirname = 'DirectoryIterator_getGroup_basic';
rmdir($dirname);
}
