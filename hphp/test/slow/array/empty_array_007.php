<?php

function a() { return array(); }
function main() {
  $x = a();
  $x['foo'][12] = 2;
  return $x;
}

<<__EntryPoint>>
function main_empty_array_007() {
var_dump(main());
}
