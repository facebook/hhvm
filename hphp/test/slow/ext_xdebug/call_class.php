<?php

function test1() {
  var_dump(xdebug_call_class());
}

function test2() {
  return test1();
}

class Foo1 {
  function test3() {
    var_dump(xdebug_call_class());
    test1();
    test2();
  }
}

class Foo2 {
  function test4() {
    var_dump(xdebug_call_class());
    (new Foo1)->test3();
  }
}

trait Bar {
  function test5() {
    var_dump(xdebug_call_class());
    $this->test4();
  }
}

class Foo3 extends Foo2 {
  use Bar;
}

var_dump(xdebug_call_class());
test1();
test2();
(new Foo1())->test3();
(new Foo2())->test4();
(new Foo3())->test5();
