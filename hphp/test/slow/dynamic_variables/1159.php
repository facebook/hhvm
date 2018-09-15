<?php

function foo($a) {
  while ($a--) {
    if (!$a) {
      var_dump(compact('x', 'y'));
    }
    $x = $a;
    $y = $a * $a;
  }
}

<<__EntryPoint>>
function main_1159() {
foo(4);
}
