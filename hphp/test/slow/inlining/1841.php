<?php

function &test(&$x) {
  $x = 1;
  return $x;
}

<<__EntryPoint>>
function main_1841() {
$x = 0;
$y = &test($x);
$y++;
var_dump($x, $y);
}
