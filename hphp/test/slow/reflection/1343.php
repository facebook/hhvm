<?php

/**
 * Doc comment on a function generator
 */
function foo() {
  yield null;
}
$rf = new ReflectionFunction('foo');
var_dump($rf->getDocComment());

class C {
  /**
   * Doc comment on a method generator
   */
  public function bar() {
    yield null;
  }
}
$rm = new ReflectionMethod('C','bar');
var_dump($rm->getDocComment());
