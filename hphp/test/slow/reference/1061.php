<?php

function run(&$a, &$b) {
  $a = 1;
  $c = $b;
  $a = 2;
  var_dump($b);
  var_dump($c);
}

<<__EntryPoint>>
function main() {
  run(&$a, &$a);
}
