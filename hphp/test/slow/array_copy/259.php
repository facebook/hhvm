<?php

function f($b) {
  $a = $b ? 0 : array('x' => $b);
  $a[0] = $a;
 var_dump($a);
}

<<__EntryPoint>>
function main_259() {
f(false);
}
