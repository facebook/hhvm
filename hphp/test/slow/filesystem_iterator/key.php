<?hh

<<__EntryPoint>>
function main_key() {
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator(
  $sample_dir,
  FilesystemIterator::KEY_AS_FILENAME
);
$ret = varray[];
foreach ($iterator as $fileinfo) {
  $ret[] = $iterator->key();
}
asort(inout $ret);
var_dump(array_values($ret));
}
