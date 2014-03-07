<?php

class A { public function yo() { echo "hi\n"; } }

function foo() {
  $x = array('x' => array(new A));
  for ($i = 0; $i < 10; ++$i) {
    $x['x'][] = new A;
  }
  return $x;
}
function main() {
  $val = foo()['x'][0];
  var_dump($val);
  $val->yo();
}
main();
