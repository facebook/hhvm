<?hh

<<__EntryPoint>>
function main_pass_self() {
$sample_dir = __DIR__.'/../../sample_dir';
$it = new FilesystemIterator($sample_dir);
$ret = varray[];
foreach ($it as $fileinfo) {
  if (is_dir((string)$fileinfo)) {
    new FilesystemIterator($fileinfo);
    var_dump(true);
  }
}
}
