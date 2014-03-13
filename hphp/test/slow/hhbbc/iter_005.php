<?php

function asd() { return array(); }
function foo() {
  $x = asd();
  foreach ($x as $v) {
    echo "unreachable code: $v\n";
  }
  return 1;
}
foo();
