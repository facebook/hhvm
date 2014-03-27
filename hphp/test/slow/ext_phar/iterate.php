<?php

function main() {
  $phar = new Phar(__DIR__.'/phpunit.phar');
  $iterator = new RecursiveIteratorIterator($phar);
  $out = array();
  foreach ($iterator as $key => $file) {
    $key = str_replace(realpath(__DIR__), '__ROOT__', $key);
    $out[$key] = array(
      $file->getFilename(),
      $file->getExtension()
    );
  }
  ksort($out);
  var_dump($out);
}

main();
