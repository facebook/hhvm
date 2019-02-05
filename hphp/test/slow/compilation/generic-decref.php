<?php

class X {
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
  for ($i = 0; $i < 101; $i++) {
    test();
  }
  var_dump(hh\objprof_get_data());
}
