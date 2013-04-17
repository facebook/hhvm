<?php

chdir(__DIR__.'/../../..');
require 'test/sample_dir/fix_mtimes.inc';

$files = array();
foreach (new DirectoryIterator('test/sample_dir/') as $file) {
  $files[] = $file;
}
var_dump(count($files));

$dir = new DirectoryIterator('test/sample_dir/');
$files = array(); // order changes per machine
foreach ($dir as $fileinfo) {
  if (!$fileinfo->isDot()) {
    $files[] = $fileinfo->getFilename();
  }
}
asort($files);
var_dump(array_values($files));

$iterator = new DirectoryIterator("test/sample_dir");
$files = array(); // order changes per machine
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $files[$fileinfo->getFilename()] = $fileinfo;
  }
}
ksort($files);
foreach ($files as $name => $fileinfo) {
  echo "BEGIN: " . $name . "\n";
  echo $fileinfo->getFilename() . "\n";
  echo $fileinfo->getCTime() . "\n";
  echo $fileinfo->getBasename() . "\n";
  echo $fileinfo->getBasename('.cpp') . "\n";
  echo $fileinfo->getGroup() . "\n";
  echo $fileinfo->getInode() . "\n";
  echo $fileinfo->getMTime() . "\n";
  echo $fileinfo->getOwner() . "\n";
  echo $fileinfo->getPerms() . "\n";
  echo $fileinfo->getSize() . "\n";
  echo $fileinfo->getType() . "\n";
  echo $fileinfo->isDir() . "\n";
  echo $fileinfo->isDot() . "\n";
  echo $fileinfo->isExecutable() . "\n";
  echo $fileinfo->isLink() . "\n";
  echo $fileinfo->isReadable() . "\n";
  echo $fileinfo->isWritable() . "\n";
  echo "END" . "\n";
}

$iterator = new RecursiveDirectoryIterator("test/sample_dir");
$files = array(); // order changes per machine
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $files[$fileinfo->getFilename()] = $fileinfo;
  }
}
ksort($files);
foreach ($files as $name => $fileinfo) {
  echo $fileinfo->getFilename() . "\n";
  echo $fileinfo->getCTime() . "\n";
  echo $fileinfo->getBasename() . "\n";
  echo $fileinfo->getBasename('.cpp') . "\n";
  echo $fileinfo->getFilename() . "\n";
  echo $fileinfo->getGroup() . "\n";
  echo $fileinfo->getInode() . "\n";
  echo $fileinfo->getMTime() . "\n";
  echo $fileinfo->getOwner() . "\n";
  echo $fileinfo->getPerms() . "\n";
  echo $fileinfo->getSize() . "\n";
  echo $fileinfo->getType() . "\n";
  echo $fileinfo->isDir() . "\n";
  echo $fileinfo->isExecutable() . "\n";
  echo $fileinfo->isLink() . "\n";
  echo $fileinfo->isReadable() . "\n";
  echo $fileinfo->isWritable() . "\n";
}
