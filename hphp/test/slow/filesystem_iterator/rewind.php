<?php
$sample_dir = __DIR__.'/../../sample_dir';
$iterator = new FilesystemIterator(
  $sample_dir,
  FilesystemIterator::KEY_AS_FILENAME
);
$a = $iterator->key();
$iterator->next();
$b = $iterator->key();

$iterator->rewind();
$c = $iterator->key();

var_dump($a == $b);
var_dump($a == $c);
