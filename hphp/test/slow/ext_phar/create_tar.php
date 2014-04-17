<?php

function run_with_iterator(Iterator $it, $root = '') {
  $tmpnam = tempnam('/tmp', 'testtar');
  unlink($tmpnam);
  $tmpnam .= '.tar';
  $pd = new PharData($tmpnam);
  var_dump($pd->buildFromIterator($it, $root));

  // Re-create for read
  $pd = new PharData($tmpnam);
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

function main() {
  run_with_iterator(new ArrayIterator(array(
    'Herp Derp' => __FILE__
  )));

  run_with_iterator(new ArrayIterator(array(
    new SplFileInfo(__FILE__)
  )), __DIR__);
}

main();
