<?php

function test() {
  $a = new DOMDocument;
  $a->loadXML('<a/>');
  $o1 = $a->getElementsByTagName('a')->item(0);
  $o2 = $a->firstChild;
  $hash1 = spl_object_hash($o1);
  $hash2 = spl_object_hash($o2);
  var_dump($hash1 === $hash2);
  return $o1;
}
function main() {
  $o1 = test();
  $o2 = test();
  $hash1 = spl_object_hash($o1);
  $hash2 = spl_object_hash($o2);
  var_dump($hash1 !== $hash2);
}
main();
