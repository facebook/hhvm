<?php


<<__EntryPoint>>
function main_1803() {
$directory = __DIR__."/../../sample_dir";
$fileSPLObjects = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($directory),
  RecursiveIteratorIterator::SELF_FIRST);
$files = array();
 // order changes per machine
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  $files[] = $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
asort($files);
var_dump(array_values($files));

$fileSPLObjects = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($directory),
  RecursiveIteratorIterator::CHILD_FIRST);
$files = array();
 // order changes per machine
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  $files[] = $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
asort($files);
var_dump(array_values($files));

$fileSPLObjects = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($directory),
  RecursiveIteratorIterator::LEAVES_ONLY);
$files = array();
 // order changes per machine
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  $files[] = $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
asort($files);
var_dump(array_values($files));

// invalid mode -100
$fileSPLObjects = new RecursiveIteratorIterator(
   new RecursiveDirectoryIterator($directory), -100);
$files = array();
 // order changes per machine
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  $files[] = $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
asort($files);
var_dump(array_values($files));

// two foreaches
$fileSPLObjects = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($directory));
$files = array();
 // order changes per machine
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  $files[] = $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {
  if (substr($fullFileName,-1)=='.') continue;
  $files[] = $fullFileName . " " .$fileSPLObject->getFilename(). "\n";
}
asort($files);
var_dump(array_values($files));
}
