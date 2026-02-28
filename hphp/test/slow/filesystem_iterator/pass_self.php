<?hh

<<__EntryPoint>>
function main_pass_self() :mixed{
$sample_dir = __DIR__.'/../../sample_dir';
$it = new FilesystemIterator($sample_dir);
$ret = vec[];
foreach ($it as $fileinfo) {
  if (is_dir((string)$fileinfo)) {
    new FilesystemIterator($fileinfo);
    var_dump(true);
  }
}
}
