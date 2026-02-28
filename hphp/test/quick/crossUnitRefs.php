<?hh

class A {
  public $foo;
  public function f() :mixed{
    inOtherUnit($this->foo, vec[]);
  }
}

<<__EntryPoint>>
function main(): void {
  include_once "crossUnitRefsInc.php";

  $a = new A();
  $a->foo = 12;
  $a->f();

  inOtherUnit(1, 2);
}
