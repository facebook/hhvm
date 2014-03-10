<?php

function foo() { return array(1,2,3); }
function bar() { return array(); }
function test($x) {
  $y = $x ? foo() : bar();
  return $y[0];
}
function main() {
  var_dump(test(true));
  var_dump(test(false));
}
main();
