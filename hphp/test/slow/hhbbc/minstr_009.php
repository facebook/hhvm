<?php

function asd() { return 'xxxxx'; }
function bar() { return mt_rand() ? 'a' : 'b'; }
function foo() {
  $x = asd();
  $x[0] = bar();
  $x[0] = 'a';
  echo $x;
  echo "\n";
  var_dump($x);
}
foo();
