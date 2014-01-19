<?php

function f($x) {
  if (!is_array($x)) {
    var_dump($x[0]);
  }
 else if (isset($x[0])) {
    var_dump($x[0]);
  }
  if (!!!is_array($x)) {
    var_dump($x[0]);
  }
 else if (isset($x[0])) {
    var_dump($x[0]);
  }
}
function g($x) {
  if (!is_array($x)) return;
  var_dump($x[0]);
}
function h($x) {
  if (!is_array($x) && !is_string($x)) {
    var_dump('1');
  }
 else {
    var_dump($x[0]);
  }
}
function i($x) {
  return !is_array($x) ? $x[0] : $x[0];
}
class X implements arrayaccess {
  private $container = array();
  public function __construct($container) {
    $this->container = $container;
  }
  public function offsetSet($offset, $value) {
    if (is_null($offset)) {
      $this->container[] = $value;
    }
 else {
      $this->container[$offset] = $value;
    }

  }
  public function offsetExists($offset) {
    return isset($this->container[$offset]);
  }
  public function offsetUnset($offset) {
    unset($this->container[$offset]);
  }
  public function offsetGet($offset) {
    return isset($this->container[$offset]) ?
      $this->container[$offset] : null;
  }
}
$x = new X(array(0, 1, 2));
f($x);
f(array(0, 1, 2));
g($x);
g(array(0, 1, 2));
h(array(0, 1, 2));
h('foobar');
h(new stdClass());
var_dump(i($x));
var_dump(i(array(0, 1, 2)));
