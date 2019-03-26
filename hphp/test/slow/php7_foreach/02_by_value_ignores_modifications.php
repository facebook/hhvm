<?php

function run(&$a) {
  foreach($a as $v) {
    echo "$v\n";
    unset($a[1]);
  }
}

<<__EntryPoint>>
function main() {
  $a = [1, 2, 3];
  run(&$a);
}
