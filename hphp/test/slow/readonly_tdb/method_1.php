<?hh
namespace ROTDB\Method_1;
class Foo {
  public int $x = 0;
  public function meth_ret_mut(): Foo {
    return new Foo();
  }
  public function meth_ret_readonly(): readonly Foo {
    return readonly new Foo();
  }
}

<<__EntryPoint>>
function main(): void {
  $foo = new Foo();
  $mut = $foo->meth_ret_mut();     // no inferred readonly
  $mut->x = 2;                     // ok
  $ro = $foo->meth_ret_readonly(); // inferred readonly
  $ro->x = 2; // ReadonlyViolationException
}
