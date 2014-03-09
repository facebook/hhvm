<?php

function x() { return 2; }
function foo() { return array(x()); }
function bar() {
  $z = foo();
  $z[1] = 12;
  for ($i = 0; $i < $z[1]; $z[1] = $z[1] - 1) {
    var_dump(is_int($z[0]));
    var_dump(is_int($z[1]));
    $z[0] = 'a';
    var_dump(is_int($z[0]));
    var_dump(is_int($z[1]));
  }
  var_dump(is_int($z[0]));
  var_dump(is_int($z[1]));
  var_dump($z);
}
bar();

