<?php

class dtor { private $i; function __construct($i) { $this->i = $i; }
                         function __destruct()    { echo "dtor $this->i\n"; } }

class heh implements ArrayAccess {
  function offsetExists($x) { return true; }
  function offsetGet($x) { return new heh(); }
  function offsetSet($x, $y) { echo "setting\n"; }
  function offsetUnset($x) {}
}

class A {
  public $z;
  function __construct() { $this->z = new heh; }
}
function x($a) {
  $a->z[new dtor(1)][new dtor(2)][new dtor(3)][new dtor(4)] = new dtor(5);
}
x(new A);
