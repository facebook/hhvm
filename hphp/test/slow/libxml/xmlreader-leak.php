<?php
// c.f. https://github.com/facebook/hhvm/issues/3899

function foo() {
  for ($i=0; $i<100; $i++) {
    $reader = new XMLReader();
    $reader->xml('<?xml version="1.0" encoding="utf-8"?><id>1234567890</id>');
    while ($reader->read()) {
      $reader->expand();
    }
  }
}

function test() {
  foo(); foo(); foo();
  $start = memory_get_peak_usage(true);
  foo();
  $end = memory_get_peak_usage(true);
  return $end - $start;
}


<<__EntryPoint>>
function main_xmlreader_leak() {
$x = test();
$x = test();
echo $x, "\n";
}
