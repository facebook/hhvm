<?php

require 'test/sample_dir/fix_mtimes.inc';

$ite=new RecursiveDirectoryIterator('test/sample_dir/');
$bytestotal=0;
$nbfiles=0;
foreach ($ite as $filename=>$cur) {
  if (substr($filename,-1)=='.') continue;
  $filesize=$cur->getSize();
  $bytestotal+=$filesize;
  $nbfiles++;
  echo "$filename => $filesize\n";
}
$bytestotal=number_format($bytestotal);
echo "Total: $nbfiles files, $bytestotal bytes\n";
