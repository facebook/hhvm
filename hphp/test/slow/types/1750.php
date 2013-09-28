<?php

function foo($a) {
  return (int)$a;
}
function test() {
  var_dump(foo(false));
}
test();
