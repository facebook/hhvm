<?php

function run(&$a, &$b, &$c, &$d) {
  $b = $d;
  var_dump($a);
  var_dump($b);
  var_dump($c);
  var_dump($d);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  $c = 2;
  run(&$a, &$a, &$c, &$c);
}
