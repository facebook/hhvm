<?php

namespace A\B;
class Foo {
  static $baz = 32;
  function __construct(array $a) {
    var_dump($a);
  }
  function callUnknownClassMethod($method) {
    return SomeUnknownClass::$method();
  }
  function unsetStaticProperty() {
    unset(Foo::$baz);
  }
}
if (rand(0, 1)) {
  class B {
    static $baz = 'baz';
    const FOO = 30;
    function f() {
      return Foo::NoSuchConstant;
    }
  }
}
 else {
  class B {
    static $baz = 'baz';
    const FOO = 30;
    function f() {
      return Foo::NoSuchConstant;
    }
  }
}
$f = new Foo(array(0));
var_dump(Foo::$baz);
var_dump(B::FOO);
var_dump(B::$baz);
