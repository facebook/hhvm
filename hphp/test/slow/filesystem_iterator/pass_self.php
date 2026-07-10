<?hh

<<__EntryPoint>>
function main_pass_self() :mixed{
$sample_dir = __DIR__.'/../../sample_dir';
$it = new FilesystemIterator($sample_dir);
$ret = vec[];
foreach ($it as $fileinfo) {
  if (is_dir($fileinfo->__toString())) {
    new FilesystemIterator($fileinfo->__toString());
    var_dump(true);
  }
}
}
