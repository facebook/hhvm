<?php
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator(
  $sample_dir,
  FilesystemIterator::CURRENT_AS_PATHNAME
);
$ret = array();
foreach ($iterator as $fileinfo) {
  $ret[] = $iterator->current();
}
asort($ret);
var_dump(array_values($ret));
