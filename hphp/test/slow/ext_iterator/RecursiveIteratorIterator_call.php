<?php


<<__EntryPoint>>
function main_recursive_iterator_iterator_call() {
$it = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator(__DIR__.'/../../sample_dir'),
  RecursiveIteratorIterator::SELF_FIRST
);
$files = array();
foreach($it as $file) {
  $files[$file->getFilename()] =
    $it->getFilename() == $file->getFilename();
}
ksort($files);
var_dump($files);
}
