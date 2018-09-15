<?php

class A {
  function foo() { return 1; }
}

function main(A $a) {
  var_dump($a->foo());
}
function handler($name, $obj, $args, $data, &$done) {
  $done = true;
  return "string!";
}


<<__EntryPoint>>
function main_intercept_methods() {
main(new A);
fb_intercept('A::foo', 'handler');
main(new A);
}
