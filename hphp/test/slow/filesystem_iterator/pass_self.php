<?php

<<__EntryPoint>>
function main_pass_self() {
$sample_dir = __DIR__.'/../../sample_dir';
$it = new FilesystemIterator($sample_dir);
$ret = array();
foreach ($it as $fileinfo) {
  if (is_dir($fileinfo)) {
    new FilesystemIterator($fileinfo);
    var_dump(true);
  }
}
}
