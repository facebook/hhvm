<?hh

class A {
  static function fun() {
    return 'A';
  }
}
class C extends B {
  public function foo() {
    $this->out(A::fun());
  }
  public function out($arg) {
    var_dump($arg);
  }
}
