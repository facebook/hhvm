<?php

function inline_me($x, $y, &$z) {
 return ($z = ($x + $y));
 }
function gen($x, $y) {
  yield inline_me($x, $y, &$arg);
  yield $arg;
}

<<__EntryPoint>>
function main_1839() {
foreach (gen(10, 20) as $x) {
 var_dump($x);
 }
}
