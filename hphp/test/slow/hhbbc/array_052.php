<?php

class A { public function yo() { echo "hi\n"; } }

function foo() {
  $x = array(array(new A));
  for ($i = 0; $i < 10; ++$i) {
    $x[] = array();
    $x[$i + 1][] = new A;
  }
  return $x;
}
function main() {
  $val = foo()[1][0];
  var_dump($val);
  $val->yo();
}

<<__EntryPoint>>
function main_array_052() {
main();
}
