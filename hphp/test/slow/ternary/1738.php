<?php

function f($x, $y) {
  return $x[0][$y++] ?: false;
}

<<__EntryPoint>>
function main_1738() {
var_dump(f(array(array(0, 1, 2)), 0));
var_dump(f(array(array(0, 1, 2)), 1));
}
