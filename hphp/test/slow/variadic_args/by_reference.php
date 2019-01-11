<?php

function variadic_by_ref(&...$args) {
  foreach ($args as $k => $v) { $args[$k]++; }
}

function main() {
  $a = 10;
  $b = 20;
  variadic_by_ref(&$a, &$b);
  var_dump($a, $b);
}

<<__EntryPoint>>
function main_by_reference() {
main();
}
