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
$obj = new A();
if (!isset($obj['a'])) {
  $obj['a'] = 'test';
}
if (!empty($obj['a'])) {
  $obj['a'] = 'test2';
}
var_dump($obj['a']);
unset($obj['a']);
var_dump($obj['a']);
