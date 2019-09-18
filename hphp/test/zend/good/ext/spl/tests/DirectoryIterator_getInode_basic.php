<?hh
<<__EntryPoint>> function main(): void {
//file
mkdir('test_dir_ptfi');
$dirIterator = new DirectoryIterator('test_dir_ptfi');
var_dump(decoct($dirIterator->getInode()));
error_reporting(0);
rmdir('test_dir_ptfi');
}
