<?php

function foo() {
  if (mt_rand() == 0) return array();
  $z = array(true, mt_rand() ? true : false);
  return $z;
}

function bar() {
  $x = foo();
  $y = $x[1];
  for ($i = 0; $i < 10; ++$i) mt_rand();
  return $y;
}

for ($i = 0; $i < 100; ++$i) {
  bar();
}
echo "Done\n";
