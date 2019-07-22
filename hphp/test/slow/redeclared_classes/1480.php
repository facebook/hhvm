<?hh

class A {
  static function fun() {
    return 'A';
  }
}
if (__hhvm_intrinsics\launder_value(true)) {
  include '1480-1.inc';
} else {
  include '1480-2.inc';
}
class C extends B {
  public function foo() {
    $this->out(A::fun());
  }
  public function out($arg) {
    var_dump($arg);
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();
  $c->foo();
}
