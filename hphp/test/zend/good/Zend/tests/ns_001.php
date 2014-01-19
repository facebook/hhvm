<?php
namespace test\ns1;

class Foo {

  function __construct() {
    echo __CLASS__,"\n";
  }
  
  function bar() {
    echo __CLASS__,"\n";
  }
  
  static function baz() {
    echo __CLASS__,"\n";
  }
}

$x = new Foo;
$x->bar();
Foo::baz();
$y = new \test\ns1\Foo;
$y->bar();
\test\ns1\Foo::baz();