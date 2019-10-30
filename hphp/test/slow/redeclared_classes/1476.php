<?hh

class PEAR {
  static function f() {
    PEAR::g();
  }
  static function g() {
    echo 'This is g()';
  }
}
if ($x) {
  include '1476.inc';
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
PEAR::f();
