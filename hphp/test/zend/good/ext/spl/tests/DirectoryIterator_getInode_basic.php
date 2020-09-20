<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
mkdir('test_dir_ptfi');
$dirIterator = new DirectoryIterator('test_dir_ptfi');
var_dump(decoct($dirIterator->getInode()));

rmdir('test_dir_ptfi');
}
