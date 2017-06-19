<?php

$dir = sys_get_temp_dir().'/hh-test-'.bin2hex(random_bytes(16));
$pd = new PharData(__DIR__.'/empty_and_modes.tar.bz2');
$pd->extractTo($dir);



$rdi = new RecursiveDirectoryIterator(
  $dir,
  FileSystemIterator::SKIP_DOTS
);
$rii = new RecursiveIteratorIterator(
  $rdi,
  RecursiveIteratorIterator::CHILD_FIRST
);

$out = [];

foreach ($rii as $path => $info) {
  $out['.'.substr($path, strlen($dir))] = [
    'mode' => decoct(stat($path)['mode']),
    'type' => $info->getType(),
  ];
  if ($info->isDir()) {
    rmdir($path);
  } else {
    unlink($path);
  }
}
rmdir($dir);
ksort($out);
var_dump($out);
