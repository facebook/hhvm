<?php

class A {}

function &bar(&$a) {
  return $a[0];
}

function main() {
  $a = array(new A);
  bar($a);
}

main();
