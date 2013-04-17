<?php

chdir(__DIR__.'/../../..');
require 'test/sample_dir/fix_mtimes.inc';

$path = "test/sample_dir/";
$files = array(); // order changes per machine
foreach (new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator($path,
      RecursiveDirectoryIterator::KEY_AS_PATHNAME),
    RecursiveIteratorIterator::CHILD_FIRST) as $file => $info) {
  if ($info->isDir() && substr($file,-1)!='.') {
    $files[] = $file."\n";
  }
}
asort($files);
var_dump(array_values($files));
