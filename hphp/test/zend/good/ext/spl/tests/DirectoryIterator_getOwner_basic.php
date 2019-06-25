<?hh <<__EntryPoint>> function main(): void {
$dirname = 'DirectoryIterator_getOwner_basic';
mkdir($dirname);
$dir = new DirectoryIterator($dirname);
$expected = fileowner($dirname);
$actual = $dir->getOwner();
var_dump($expected == $actual);
error_reporting(0);
$dirname = 'DirectoryIterator_getOwner_basic';
rmdir($dirname);
}
