<?php

class Base {
  function foo() {
    $m = new ReflectionMethod(get_class($this), 'bar');
    var_dump($m->name);
  }
}
$condition = 123;
if ($condition) {
  class A extends Base {
}
}
 else {
  class A extends Base {
}
}
class B extends A {
  function bar() {
  }
}
$obj = new B();
$obj->foo();
