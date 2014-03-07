<?php

class X implements ArrayAccess {
  function offsetExists($offset) { return false; }
  function offsetGet($offset) { return 1; }
  function offsetSet($offset, $value) { throw new Exception(); }
  function offsetUnset($offset) { return; }
}

function foo() {
  $x[0] = new X;
  try {
    $x[0]['asd'] = 2;
  } catch (Exception $e) {
    var_dump(is_array($x));
    var_dump(is_object($x[0]));
    var_dump($x);
  }
}
foo();
