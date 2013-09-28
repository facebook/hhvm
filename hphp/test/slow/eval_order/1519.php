<?php

class C {
  function __construct() {
    echo "class C\n";
  }
  public function __get( $what ) {
    echo "get C\n";
    return $this->_p[ $what ];
  }
  public function __set( $what, $value ) {
    echo "set C\n";
    $this->_p[ $what ] = $value;
  }
  private $_p = array();
}
function f() {
  echo "f()\n";
  return 1;
}
function foo() {
  $obj = new C;
  $obj->a = f();
  $obj->b = new C;
  $obj->b->a = f();
}
foo();
