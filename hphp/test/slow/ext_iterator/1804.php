<?php

function getFiles(&$rdi,$depth=0) {
  if (!is_object($rdi)) return;
  $files = array();
  // order changes per machine
  for ($rdi->rewind(); $rdi->valid(); $rdi->next()) {
    if ($rdi->isDot()) continue;
    if ($rdi->isDir() || $rdi->isFile()) {
      $indent = '';
      for ($i = 0; $i<=$depth; ++$i) $indent .= " ";
      $files[] = $indent.$rdi->current()."\n";
      if ($rdi->hasChildren()) getFiles($rdi->getChildren(),1+$depth);
    }
  }
  asort($files);
  var_dump(array_values($files));
}
getFiles(new RecursiveDirectoryIterator(__DIR__.'/../../sample_dir'));
