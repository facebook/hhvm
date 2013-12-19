<?php

$iter = new GlobIterator(__DIR__ . "/../../sample_dir/*");

var_dump($iter->count());
var_dump($iter->getPath());
var_dump($iter->getPathname());
var_dump($iter->getFilename());

foreach ($iter as $file) {
  echo "$file\n";
}
