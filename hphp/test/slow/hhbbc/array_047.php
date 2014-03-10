<?php

class A { public function yo() { echo "hi\n"; } }

function foo() {
  $x = array(new A);
  for ($i = 0; $i < 10; ++$i) {
    $x[] = new A;
  }
  return $x;
}
function main() {
  $val = foo()[1];
  var_dump($val);
  $val->yo();
}
main();
