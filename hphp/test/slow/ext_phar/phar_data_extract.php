<?php

function tempdir($dir=false,$prefix='php') {
  $tempfile = tempnam(sys_get_temp_dir(),'');
  if (file_exists($tempfile)) {
    unlink($tempfile);
  }
  mkdir($tempfile);
  if (is_dir($tempfile)) {
    return $tempfile;
  }
}

function getFileCount($path) {
  $size = 0;
  $files = scandir($path);
  foreach($files as $t) {
    if ($t[0] == '.') {
      continue;
    }
    $name = rtrim($path, '/') . '/' . $t;
    if (is_dir($name)) {
      $size += getFileCount(rtrim($path, '/') . '/' . $t);
    } else {
      $size++;
      unlink($name);
    }
  }
  return $size;
}

$directory = dirname(__FILE__)."/tgz";
$iterator = new DirectoryIterator($directory);
$files = array();
foreach ($iterator as $fileinfo) {
  if ($fileinfo->isFile()) {
    $files[$fileinfo->getPathname()] = $fileinfo->getFilename();
  }
}

ksort($files);
foreach ($files as $path => $file) {
  $p = new PharData($path);
  $tmpdir = tempdir();
  $p->extractTo($tmpdir);

  var_dump($file);
  var_dump(getFileCount($tmpdir));
  rmdir($tmpdir);
}
