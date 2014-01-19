<?php

$dir = new DirectoryIterator(__DIR__.'/../../sample_dir');
$files = array();
 // order changes per machine
while($dir->valid()) {
  if(!$dir->isDot()) {
    $files[] = $dir->current()."\n";
  }
  $dir->next();
}
asort($files);
var_dump(array_values($files));
