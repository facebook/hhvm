<?php
$sample_dir = __DIR__.'/../../sample_dir';
$it = new FilesystemIterator($sample_dir);
$ret = array();
foreach ($it as $fileinfo) {
  $ret[] = $fileinfo->getFilename();
}
asort($ret);
var_dump(array_values($ret));
