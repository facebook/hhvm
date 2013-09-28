<?php

function foo($t) {
  $x = function() use ($t) {
    var_dump($t);
    yield 1;
  }
;
  foreach ($x() as $y) {
    var_dump($y);
  }
}
foo(42);
