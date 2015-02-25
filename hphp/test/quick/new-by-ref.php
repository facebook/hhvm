<?php


class C{
  public function __construct($v) {
    $this->prop = $v;
  }
}

function &blah() {
  $x = &new C(rand());
  return $x;
}

var_dump(blah());
