<?php

function a() { return array(); }
function main() {
  $x = a();
  $x[] = 2;
  return $x;
}


<<__EntryPoint>>
function main_empty_array_001() {
var_dump(main());
}
