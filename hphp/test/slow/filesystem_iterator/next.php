<?php
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator($sample_dir);
$ret = array();
while($iterator->valid()) {
  $ret[] = $iterator->getFilename();
  $iterator->next();
}
asort($ret);
var_dump(array_values($ret));
