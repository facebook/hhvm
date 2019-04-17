<?php

function run(&$a, &$c) {
  $b = array(0, 1);
  $a = array($b, 1);
  unset($a[0][0]);
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  run(&$a, &$a);
}
