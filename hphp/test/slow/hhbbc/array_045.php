<?php

function foo() {
  return mt_rand() ? array(1,2,3) : array(2,3,4);
}
function bar() {
  $x = foo();
  $x[123] = 2;
  $x['asdasdasd'] = new stdclass;
  return $x;
}
function main() {
  $x = bar();
  var_dump($x['asdasdasd']);
}
main();
