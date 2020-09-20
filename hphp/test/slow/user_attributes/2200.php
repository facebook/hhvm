<?hh

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


<<__EntryPoint>>
function main_2200() {
$rm = new ReflectionMethod('F', 'foo');

var_dump($rm->getAttribute('W'));
var_dump($rm->getAttribute('X'));
var_dump($rm->getAttribute('Y'));
var_dump($rm->getAttribute('Z'));

$attrs = $rm->getAttributes();
ksort(inout $attrs);
var_dump($attrs);
}
