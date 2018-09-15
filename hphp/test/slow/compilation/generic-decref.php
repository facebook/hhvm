<?php

class X {
  function __destruct() { echo "dead\n"; }
}
function thing() {
  static $s = 0;
  if (!($s++ % 100)) return new X;
  return 42;
}

function test() {
  thing();
}

<<__EntryPoint>>
function main_generic_decref() {
;

for ($i = 0; $i < 101; $i++) {
  test();
}
}
