<?php

class dtor {
  private $i;
  function __construct($i) { $this->i = $i; }
  function __destruct()    { echo "dtor $this->i\n"; }
  function __toString() { return $this->i == 4 ? 'x' : 'z'; }
}

class heh {
  private $i = 0;
  function __get($x) {
    if ($x == 'x') return "asd";
    return new heh;
  }
}

class A {
  public $z;
  function __construct() { $this->z = new heh; }
}
function x($a) {
  // This is going to take the strTestResult branch.
  var_dump(
    $a->{new dtor(1)}->{new dtor(2)}->{new dtor(3)}->{new dtor(4)}[0]
      = new dtor(5)
  );
}
x(new A);
