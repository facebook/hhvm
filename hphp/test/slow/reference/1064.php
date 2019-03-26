<?php

function run(&$a, &$b) {
  $c = $b;
  $b = 2;
  var_dump($a);
  var_dump($c);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  run(&$a, &$a);
}
