<?hh

class PEAR {
  static function f() {
    PEAR::g();
  }
  static function g() {
    echo 'This is g()';
  }
}
class D1 extends PEAR {
  public $foo;
  private $bar;
  function bar() {
    return $this->foo + $this->bar;
  }
}
class D2 extends D1 {
  public $foo;
  private $bar;
  function bar() {
    return $this->foo + $this->bar;
  }
}
<<__EntryPoint>>
function entrypoint_1476(): void {
  PEAR::f();
}
