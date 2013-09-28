<?php
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator(
  $sample_dir,
  FilesystemIterator::KEY_AS_FILENAME
);
$ret = array();
foreach ($iterator as $fileinfo) {
  $ret[] = $iterator->key();
}
asort($ret);
var_dump(array_values($ret));
