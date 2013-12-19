<?php

$path = 'glob://' . __DIR__ . '/../../sample_dir/*';

$iters = array(
  new DirectoryIterator($path),
  new FilesystemIterator($path),
  new GlobIterator($path)
);

foreach ($iters as $iter) {
  var_dump(get_class($iter));
  var_dump($iter->getPath());
  var_dump($iter->getPathname());
  var_dump($iter->getFilename());

  foreach ($iter as $file) {
    echo "$file\n";
  }
}
