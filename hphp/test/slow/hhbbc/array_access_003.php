<?php

class X implements ArrayAccess {
  function offsetExists($offset) { return false; }
  function offsetGet($offset) { return 1; }
  function offsetSet($offset, $value) { return 1; }
  function offsetUnset($offset) { return; }
}

function bar() { return mt_rand() ? array(new X) : array("ASD"); }
function foo() {
  $x = bar(); // $x: Arr(InitCell)
  $x[0] = 'heh';
  $y = $x[0];
  var_dump(is_string($y)); // true
}

foo();
