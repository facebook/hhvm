<?

include_once "crossUnitRefsInc.php";


class A {
  public $foo;
  public function f() {
    inOtherUnit($this->foo, array());
  }
}

$a = new A();
$a->foo = 12;
$a->f();

inOtherUnit(1, 2);
