<?php

class A implements ArrayAccess {
  public $a;
  public function offsetExists($offset) {
    echo "offsetExist";
    return false;
  }
  public function offsetGet($offset) {
    echo "offsetGet";
    return $this->$offset.'get';
  }
  public function offsetSet($offset, $value) {
    $this->$offset = $value.'set';
  }
  public function offsetUnset($offset) {
    $this->$offset = 'unset';
  }
}
function f() {
 var_dump('f()');
 return 1;
 }
function test($a) {
$a['foo'] .= f();
$a['bar'] += f();
$a['bar'] -= f();
$a['bar'] *= f();
$a['bar'] /= f();
$a['bar'] %= f();
$a['bar'] &= f();
$a['bar'] |= f();
$a['bar'] ^= f();
$a['bar'] <<= f();
$a['bar'] >>= f();
}
test(new A);
