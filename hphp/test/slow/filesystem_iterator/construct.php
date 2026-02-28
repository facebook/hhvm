<?hh

<<__EntryPoint>>
function main_construct() :mixed{
$sample_dir = __DIR__.'/../../sample_dir';
$it = new FilesystemIterator($sample_dir);
$ret = vec[];
foreach ($it as $fileinfo) {
  $ret[] = $fileinfo->getFilename();
}
asort(inout $ret);
var_dump(array_values($ret));
}
