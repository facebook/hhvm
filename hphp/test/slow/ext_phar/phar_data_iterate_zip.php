<?php

function main() {
  $pd = new PharData(__DIR__.'/archive.zip');
  $out = array();
  $it = new RecursiveIteratorIterator($pd);
  foreach ($it as $fname => $data) {
    $out[$fname] = array(
      get_class($data),
      $data->getFilename(),
      $data->getPathname(),
      (string) $data
    );
  }
  ksort($out);
  var_dump($out);
}

main();
