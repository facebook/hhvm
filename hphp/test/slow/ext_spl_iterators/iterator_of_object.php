<?hh


<<__EntryPoint>>
function main_iterator_of_object() {
$info = new SplFileInfo(sys_get_temp_dir());
$it = new FilesystemIterator($info);
var_dump($it->getPath() === sys_get_temp_dir());
}
