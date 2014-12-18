<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class C
{
  public function __get($name) {
    echo "get\n";
    return $this->$name;    // must not recurse
  }

  public function __set($name, $val) {
    echo "set\n";
    $this->$name = $val;    // must not recurse
  }

  public function __isset($name) {
    echo "isset\n";
    return isset($this->$name);    // must not recurse
  }

  public function __unset($name) {
    echo "unset\n";
    unset($this->$name);    // must not recurse
  }
}

$c = new C;
$x = $c->prop;  // Undefined property: C::$prop
$c->prop = 123; // Defined now
$x = $c->prop;
var_dump($x);
var_dump($c);
