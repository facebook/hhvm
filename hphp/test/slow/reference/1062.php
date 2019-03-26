<?php

function run(&$a, &$b) {
  $b = 2;
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  $a = 1;
  run(&$a, &$a);
}
