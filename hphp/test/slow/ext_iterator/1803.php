<?php

require 'test/sample_dir/fix_mtimes.inc';

$directory = "test/sample_dir";
$fileSPLObjects =  new RecursiveIteratorIterator(  new RecursiveDirectoryIterator($directory),  RecursiveIteratorIterator::SELF_FIRST);
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  print $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
$fileSPLObjects =  new RecursiveIteratorIterator(  new RecursiveDirectoryIterator($directory),  RecursiveIteratorIterator::CHILD_FIRST);
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  print $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
$fileSPLObjects =  new RecursiveIteratorIterator(  new RecursiveDirectoryIterator($directory),  RecursiveIteratorIterator::LEAVES_ONLY);
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  print $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
// invalid mode -100$fileSPLObjects =  new RecursiveIteratorIterator(  new RecursiveDirectoryIterator($directory), -100);
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  print $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
