<?php

include __DIR__.'/../../../test/sample_dir/fix_mtimes.inc';

$dir = new DirectoryIterator('test/sample_dir');
$files = array(); // order changes per machine
while($dir->valid()) {
  if(!$dir->isDot()) {
    $files[] = $dir->current()."\n";
  }
  $dir->next();
}
asort($files);
var_dump(array_values($files));
