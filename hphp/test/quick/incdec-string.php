<?php

function foo() {
  $go = function($start) {
    $x = $start;
    for ($i = 0; $i < 10000; ++$i) {
      ++$x;
      if (($i % 101) == 0) { var_dump($x); var_dump(strlen($x)); }
    }
  };
  $go('a');
  $go('A');
  $go('A1');
  $go('a1');
}
foo();

function bar() {
  $x = '1';
  var_dump(is_int(++$x));
}
bar();
