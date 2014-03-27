<?php

class A {
  function foo() { return 1; }
}

function main(A $a) {
  var_dump($a->foo());
}

main(new A);
function handler($name, $obj, $args, $data, &$done) {
  $done = true;
  return "string!";
}
fb_intercept('A::foo', 'handler');
main(new A);
