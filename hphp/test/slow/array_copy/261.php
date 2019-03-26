<?php

function f($b) {
  $a = $b ? 0 : array('x' => $b);
  $a[] = $a;
  var_dump($a);
}

<<__EntryPoint>>
function main_261() {
f(false);
}
