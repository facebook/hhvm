<?php

require 'test/sample_dir/fix_mtimes.inc';

function getFiles(&$rdi,$depth=0) {
  if (!is_object($rdi)) return;
  for ($rdi->rewind(); $rdi->valid(); $rdi->next()) {
    if ($rdi->isDot()) continue;
    if ($rdi->isDir() || $rdi->isFile()) {
      for ($i = 0; $i<=$depth; ++$i) echo " ";
      echo $rdi->current()."\n";
      if ($rdi->hasChildren()) getFiles($rdi->getChildren(),1+$depth);
    }
  }
}
getFiles(new RecursiveDirectoryIterator('test/sample_dir'));
