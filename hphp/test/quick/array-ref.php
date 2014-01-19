<?php

function yes() { return true; }

function main() {
  $a = array();
  $a['wat'] =& $a;
  $b = $a; // Make sure the next line triggers COW
  if (yes()) {
    // Force a new tracelet
    $a['wat'] = 5;
  }
  var_dump($a);
}
main();

function main2(&$a) {
  $a = array();
  $a['foo'] = 'flee';
}
main2($z);
var_dump($z);
