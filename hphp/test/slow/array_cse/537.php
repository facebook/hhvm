<?php

class ArrayWrap implements arrayaccess {
  private $x;
  public function __construct($x) {
    $this->x = $x;
  }
  public function offsetSet($offset, $value) {
    $this->x[$offset] = $value;
  }
  public function offsetExists($offset) {
    return isset($this->x[$offset]);
  }
  public function offsetUnset($offset) {
    unset($this->x[$offset]);
  }
  public function offsetGet($offset) {
    return $this->x[$offset];
  }
}

function f1($x) {
  return isset($x[0]) && $x[0];
}

function f2($x) {
  if (!is_null($x[0])) var_dump($x[0]);
  var_dump($x[0]);
}

function f3($x) {
  foreach ($x['foo'] as $k => $v) {
    if ($v) unset($x['foo'][$k]);
  }
  var_dump($x);
}

function f4($x) {
  var_dump($x[0][1]);
  unset($x[0][1]);
  var_dump($x[0][1]);
}

function f5($x) {
  var_dump(md5($x[0]), $x[0]);
}


<<__EntryPoint>>
function main_537() {
error_reporting(0);
$o = new ArrayWrap(array(0, 1, 2));
var_dump(f1(null));
var_dump(f1(array()));
var_dump(f1(array(0)));
var_dump(f1(''));
var_dump(f1('a'));
var_dump(f1($o));
f2(array(0 => array()));
f2(array());
f2('');
f2($o);
f2(null);
f3(array('foo' => array(0,1,2,3)));
f4(array(array(1 => new stdClass())));
f5('foobar');
}
