<?hh

include_once "crossUnitRefsInc.php";


class A {
  public $foo;
  public function f() {
    inOtherUnit($this->foo, varray[]);
  }
}

<<__EntryPoint>>
function main(): void {
  $a = new A();
  $a->foo = 12;
  $a->f();

  inOtherUnit(1, 2);
}
