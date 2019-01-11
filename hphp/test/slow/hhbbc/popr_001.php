<?php

class A {}

function bar(&$a) {
  return $a[0];
}

function main() {
  $a = array(new A);
  bar(&$a);
}


<<__EntryPoint>>
function main_popr_001() {
main();
}
