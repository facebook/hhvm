<?php

$sample_dir = __DIR__.'/../../sample_dir';

$files = array();
foreach (new DirectoryIterator($sample_dir.'/') as $file) {
  $files[] = $file;
}
var_dump(count($files));

$dir = new DirectoryIterator($sample_dir.'/');
$files = array();
 // order changes per machine
foreach ($dir as $fileinfo) {
  if (!$fileinfo->isDot()) {
    $files[] = $fileinfo->getFilename();
  }
}
asort($files);
var_dump(array_values($files));

$iterator = new DirectoryIterator($sample_dir);
$files = array();
 // order changes per machine
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $str = "BEGIN:\n";
    $name = $fileinfo->getFilename();
    $str .= $name . "\n";
    $fileinfo->getCTime() . "\n";
    $fileinfo->getBasename() . "\n";
    $fileinfo->getBasename('.cpp') . "\n";
    $fileinfo->getGroup() . "\n";
    $fileinfo->getInode() . "\n";
    $fileinfo->getMTime() . "\n";
    $fileinfo->getOwner() . "\n";
    $fileinfo->getPerms() . "\n";
    $fileinfo->getSize() . "\n";
    $fileinfo->getType() . "\n";
    $fileinfo->isDir() . "\n";
    $fileinfo->isDot() . "\n";
    $fileinfo->isExecutable() . "\n";
    $fileinfo->isLink() . "\n";
    $fileinfo->isReadable() . "\n";
    $fileinfo->isWritable() . "\n";
    $str .= "END\n";
    $files[$name] = $str;
  }
}
ksort($files);
foreach ($files as $str) {
  echo $str;
}

$iterator = new RecursiveDirectoryIterator($sample_dir);
$files = array();
 // order changes per machine
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $files[$fileinfo->getFilename()] = $fileinfo;
  }
}
ksort($files);
foreach ($files as $name => $fileinfo) {
  echo $fileinfo->getFilename() . "\n";
   $fileinfo->getCTime() . "\n";
   $fileinfo->getBasename() . "\n";
   $fileinfo->getBasename('.cpp') . "\n";
   $fileinfo->getFilename() . "\n";
   $fileinfo->getGroup() . "\n";
   $fileinfo->getInode() . "\n";
   $fileinfo->getMTime() . "\n";
   $fileinfo->getOwner() . "\n";
   $fileinfo->getPerms() . "\n";
   $fileinfo->getSize() . "\n";
   $fileinfo->getType() . "\n";
   $fileinfo->isDir() . "\n";
   $fileinfo->isExecutable() . "\n";
   $fileinfo->isLink() . "\n";
   $fileinfo->isReadable() . "\n";
   $fileinfo->isWritable() . "\n";
}
