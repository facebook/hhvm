<?php

error_reporting(-1);

class Foo {
  static $bar = array('baz' => 'myfunc');
}

function myfunc() {
  return 'quux';
}

var_dump(Foo::$bar['baz']());
