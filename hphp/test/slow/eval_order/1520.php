<?php

class C implements ArrayAccess {
  private $data = array();
  public function __construct() {
    echo "C\n";
  }
  public function offsetGet($name) {
    echo "offsetGet: $name\n";
    return $this->data[$name];
  }
  public function offsetSet($name, $value) {
    $a = serialize($value);
    echo "offsetSet: $name=$a\n";
    $this->data[$name]=$value;
  }
  public function offsetExists($name) {
    echo "offsetExists: $name\n";
 return true;
  }
  public function offsetUnset($name) {
    echo "offsetUnset: $name\n";
  }
}
function f() {
  echo "f()\n";
  return 1;
}
function f2() {
  echo "f2()\n";
  return 'foo';
}
function foo($a) {
  $a['foo'] = new C;
  $a['foo']['bar'] = new C;
  $a['foo']['bar']['goo'] = f();
}
foo(new C);
