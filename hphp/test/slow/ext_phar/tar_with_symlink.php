<?php

$pd = new PharData(__DIR__.'/tgz/with_symlink.tar.gz');

$tempdir = tempnam(sys_get_temp_dir(),'');
unlink($tempdir);
mkdir($tempdir);

$pd->extractTo($tempdir);
$fi = new SplFileInfo($tempdir.'/foo/herp');
var_dump($fi->isLink());
var_dump($fi->getLinkTarget());

$rdi = new RecursiveDirectoryIterator(
  $tempdir,
  FileSystemIterator::SKIP_DOTS
);
$rii = new RecursiveIteratorIterator(
  $rdi,
  RecursiveIteratorIterator::CHILD_FIRST
);

foreach ($rii as $path => $info) {
  if ($info->isDir()) {
    rmdir($path);
  } else {
    unlink($path);
  }
}
rmdir($tempdir);
