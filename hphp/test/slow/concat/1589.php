<?php

function foo() {
  print " FOO ";
  return " foo ";
}
class A implements ArrayAccess {
  private $data = array();
  public function offsetUnset($index) {
}
  public function offsetGet($index) {
    print " GET ";
    return " get ";
  }
  public function offsetSet($index, $value) {
    $data[$index] = $value;
  }
  public function offsetExists($index) {
 }
}
class C {
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
function bar() {
  print " hello " . foo() . "\n";
  $a = new A;
  $a[0] = 0;
  $a[1] = 1;
  echo " hello $a[0]";
  echo " hello $a[1]\n";
  print " hello $a[0]";
  print " hello $a[1]\n";
  $b = new C;
  $b->a = 'aaaa';
  $b->b = 'bbbb';
  echo " hello $b->a";
  echo " hello $b->b\n";
  print " hello $b->a";
  print " hello $b->b\n";
  echo " hello $b->a $b->b $b->a $b->b";
}
bar();
