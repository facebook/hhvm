<?php

function test() {
  $a = new DOMDocument;
  $a->loadXML('<a/>');
  $hash1 = spl_object_hash($a->getElementsByTagName('a')->item(0));
  $hash2 = spl_object_hash($a->firstChild);
  var_dump($hash1 === $hash2);
  return $hash1;
}
function main() {
  $hash1 = test();
  $hash2 = test();
  var_dump($hash1 !== $hash2);
}
main();
