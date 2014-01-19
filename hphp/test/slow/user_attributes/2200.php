<?php

class A {
  <<W(1),X(2)>>
  private function foo() {
}
}
class B extends A {
}
class C extends B {
  <<X(3),Y(4)>>
  protected function foo() {
}
}
class D extends C {
}
class E extends D {
  <<Y(5),Z(6)>>
  public function foo() {
}
}
class F extends E {
}

$rm = new ReflectionMethod('F', 'foo');

var_dump($rm->getAttribute('W'));
var_dump($rm->getAttribute('X'));
var_dump($rm->getAttribute('Y'));
var_dump($rm->getAttribute('Z'));

$attrs = $rm->getAttributes();
ksort($attrs);
var_dump($attrs);

var_dump($rm->getAttributeRecursive('W'));
var_dump($rm->getAttributeRecursive('X'));
var_dump($rm->getAttributeRecursive('Y'));
var_dump($rm->getAttributeRecursive('Z'));

$attrs = $rm->getAttributesRecursive();
ksort($attrs);
var_dump($attrs);
