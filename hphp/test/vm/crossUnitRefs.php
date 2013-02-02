<?

include_once "crossUnitRefsInc.php";


class A {
  public $foo;
  public function f() {
    inOtherUnit($this->foo, array());
  }
}

function main() {
  $a = new A();
  $a->foo = 12;
  $a->f();

  inOtherUnit(1, 2);
}

main();

