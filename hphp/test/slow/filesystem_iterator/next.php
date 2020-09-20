<?hh

<<__EntryPoint>>
function main_next() {
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator($sample_dir);
$ret = varray[];
while($iterator->valid()) {
  $ret[] = $iterator->getFilename();
  $iterator->next();
}
asort(inout $ret);
var_dump(array_values($ret));
}
