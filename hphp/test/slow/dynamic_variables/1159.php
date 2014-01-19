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
foo(4);
