<?php

function asd() { return array(); }
function foo() {
  $x = asd();
  foreach ($x as $k => $v) {
    echo "unreachable code: $k $v\n";
  }
  return 1;
}
foo();
