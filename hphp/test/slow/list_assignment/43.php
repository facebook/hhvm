<?php

class obj implements arrayaccess {
    private $container = array();
    public function __construct() {
        $this->container = array(            'one'   => 1,            'two'   => 2,            'three' => 3,        );
    }
    public function offsetSet($offset, $value) {
        $this->container[$offset] = $value;
    }
    public function offsetExists($offset) {
        return isset($this->container[$offset]);
    }
    public function offsetUnset($offset) {
        unset($this->container[$offset]);
    }
    public function offsetGet($offset) {
        return isset($this->container[$offset]) ? $this->container[$offset] : null;
    }
}
class SetTest {
  private $_vals = array(      'one'   => 1,      'two'   => 2,      'three' => 3,      );
  public function __set($name, $value) {
    $this->_vals[$name] = $value;
  }
}
$o = new obj;
$q = list($o['one'], $o['two'], list($o['three'])) =  array('eins', 'zwei', array('drei'));
var_dump($o);
var_dump($q);
$x = new SetTest;
$qq = list($x->one, $x->two, list($x->three)) = 1;
var_dump($x);
$qq = list($x->one, $x->two, list($x->three)) = $q;
var_dump($x);
var_dump($qq);
