<?php

require 'test/sample_dir/fix_mtimes.inc';

$files = array();
foreach (new DirectoryIterator('test/sample_dir/') as $file) {
  $files[] = $file;
}
var_dump(count($files));
$dir = new DirectoryIterator('test/sample_dir/');
foreach ($dir as $fileinfo) {
  if (!$fileinfo->isDot()) {
    var_dump($fileinfo->getFilename());
  }
}
$iterator = new DirectoryIterator("test/sample_dir");
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    echo "BEGIN: " . $fileinfo->getFilename() . "\n";
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
}
$iterator = new RecursiveDirectoryIterator("test/sample_dir");
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
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
}

