<?php

function run_with_iterator(Iterator $it, $root = '') {
  $tmpnam = tempnam('/tmp', 'testzip');
  unlink($tmpnam);
  $tmpnam .= '.zip';
  $pd = new PharData($tmpnam);
  var_dump($pd->buildFromIterator($it, $root));
  $z = zip_open($tmpnam);
  while ($entry = zip_read($z)) {
    var_dump([zip_entry_name($entry), zip_entry_filesize($entry)]);
  }
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
