<?hh
namespace ROTDB\PrimitivesNamespacing;
class Foo {
  public int $x = 0;
}

function ro(): int {
  $foo = readonly new Foo();
  // There should be no error on the next line. Ensures we handle namespacing of primitive types correctly
  return (readonly $foo->x) as int;
}
