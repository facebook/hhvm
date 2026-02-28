<?hh

<<__EntryPoint>>
function main_set_flags() :mixed{
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator(
  $sample_dir,
  FilesystemIterator::KEY_AS_PATHNAME
);
echo "Key as Pathname:\n";
$ret = vec[];
foreach ($iterator as $key => $fileinfo) {
  $ret[] = $key;
}
asort(inout $ret);
var_dump(array_values($ret));

$iterator->setFlags(FilesystemIterator::KEY_AS_FILENAME);
echo "\nKey as Filename:\n";
$ret = vec[];
foreach ($iterator as $key => $fileinfo) {
  $ret[] = $key;
}
asort(inout $ret);
var_dump(array_values($ret));
}
