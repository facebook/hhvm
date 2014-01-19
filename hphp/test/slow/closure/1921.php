<?php

function foo() {
  $var = 123;
  $ref = 456;
  $abc = 789;
  $a = function () use ($var, &$ref) {
    var_dump($abc, $var, $ref);
    $abc = $var = $ref = 333;
  }
;
  var_dump($a());
  var_dump($abc, $var, $ref);
}
foo();
