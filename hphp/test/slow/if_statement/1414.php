<?php

function test($a,$b = 0) {
  if ($a == 2) {
    if ($b == 1) {
      return;
    }
    $a = 5;
  }
  if ($a == 3) {
    var_dump($a);
  }
}

<<__EntryPoint>>
function main_1414() {
test(3);
}
