<?php

function err() { echo "err\n"; }
set_error_handler('err');

class dtor implements ArrayAccess {
  private $num;
  function __construct($num) { $this->num = $num; }
  function __destruct() { echo "dtor: $this->num\n"; }

  function offsetExists($x) { return true; }
  function offsetGet($x) { return array(); }
  function offsetSet($x, $y) {
    echo "set\n";
    throw new exception();
  }
  function offsetUnset($x) {}
  public $asd = array(array(0, 1, 2));
}

class heh implements ArrayAccess {
  function offsetExists($x) { return true; }
  function offsetGet($x) { return new dtor(1); }
  function offsetSet($x, $y) {}
  function offsetUnset($x) {}
}

function get() { echo "get\n"; return 0; }

function heh2($x, $foo) {
  try {
    return $x[$foo]->asd[0][new dtor(2)] = new dtor(3);
  } catch (exception $x) { return null; }
}

var_dump(heh2(new heh, 0));
echo "ok\n";
