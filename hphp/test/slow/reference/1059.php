<?php

function run(&$a, &$b) {
  $a = 10;
  var_dump($b);
}

<<__EntryPoint>>
function main() {
  run(&$a, &$a);
}
