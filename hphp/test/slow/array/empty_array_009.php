<?php

function a() { return array(); }
function main() {
  $x = a();
  return $x + array(1,2,3);
}

<<__EntryPoint>>
function main_empty_array_009() {
var_dump(main());
}
