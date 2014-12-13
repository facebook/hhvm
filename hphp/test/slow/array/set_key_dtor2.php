<?php

class dtor {
  private $i;
  function __construct($i) { $this->i = $i; }
  function __destruct()    { echo "dtor $this->i\n"; }
  function __toString() { return 'z'; }
}

class heh {
  function __get($x) { return new heh; }
}

class A {
  public $z;
  function __construct() { $this->z = new heh; }
}
function x($a) {
  $a->{new dtor(1)}->{new dtor(2)}->{new dtor(3)}->{new dtor(4)} = new dtor(5);
}
x(new A);
