<?php

class stringer {
  public function __toString() { throw new Exception("nope\n"); }
}

function foo() {
  $x = new stringer();
  $b = "foo";
  $b .= $x;
}

try {
  foo();
} catch (Exception $e) {
}

print "all ok\n";
