<?php

function run(&$a, &$b) {
  $c = 2;
  $b = $c;
  $c = 5;
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  run(&$a, &$a);
}
