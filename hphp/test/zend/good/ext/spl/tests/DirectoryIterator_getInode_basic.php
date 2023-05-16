<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
mkdir('test_dir_ptfi');
$dirIterator = new DirectoryIterator('test_dir_ptfi');
var_dump(decoct($dirIterator->getInode()));

rmdir('test_dir_ptfi');
}
