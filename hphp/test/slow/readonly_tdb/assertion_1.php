<?hh
namespace ROTDB\Assertion1;

class Foo {
  public function ro(): readonly int {
    return 1;
  }
}

<<__EntryPoint>>
function main(): void {
  $foo = new Foo();
  $foo as dynamic;
  $foo->ro();
}
