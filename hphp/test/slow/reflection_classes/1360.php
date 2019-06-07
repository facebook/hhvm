<?hh

class Base {
  function foo() {
    $m = new ReflectionMethod(get_class($this), 'bar');
    var_dump($m->name);
  }
}
$condition = 123;
if ($condition) {
  include '1360-1.inc';
} else {
  include '1360-2.inc';
}
class B extends A {
  function bar() {
  }
}
$obj = new B();
$obj->foo();
