<?php

function f($b) {
  $a = $b ? 0 : array('x' => $b);
  $a2 = &$a;
 $a[] = $a2;
 var_dump($a);
}

<<__EntryPoint>>
function main_261() {
f(false);
}
