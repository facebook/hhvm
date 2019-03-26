<?php

function foo(&$a, &$b) {
  $b = 1;
  yield $a;
  $a = 3;
  $b = 2;
  yield $a;
}

<<__EntryPoint>>
function main_1848() {
  foreach (foo(&$a, &$a) as $x) var_dump($x);
}
