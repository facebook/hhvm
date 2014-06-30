<?php

function foo($a, $b, $c, $d, $e, $f, $g) {
  var_dump(debug_backtrace());
  throw new Exception;
}
class A {
  public function __toString() {
    return 'a';
  }
}
foo(10, 'a', new stdclass, new A, [20], 30, null);
