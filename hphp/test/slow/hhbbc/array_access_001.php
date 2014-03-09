<?php

class X implements ArrayAccess {
  function offsetExists($offset) { return false; }
  function offsetGet($offset) { return 1; }
  function offsetSet($offset, $value) { return 1; }
  function offsetUnset($offset) { return; }
}

function bar() { return array(new X); }
function foo() {
  $x = bar();
  $x[0][1] = 'heh';
  $y = $x[0][1];
  var_dump(is_string($y));
}

foo();
