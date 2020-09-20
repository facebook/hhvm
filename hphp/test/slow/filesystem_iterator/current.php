<?hh

<<__EntryPoint>>
function main_current() {
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator(
  $sample_dir,
  FilesystemIterator::CURRENT_AS_PATHNAME
);
$ret = varray[];
foreach ($iterator as $fileinfo) {
  $ret[] = $iterator->current();
}
asort(inout $ret);
var_dump(array_values($ret));
}
