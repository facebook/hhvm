<?hh // strict
class FooBase {}
class Foo extends FooBase {}
class FooDerived extends Foo {}
class BlahBase {}
class Blah extends BlahBase {}
class BlahDerived extends Blah {}
function bar<Tx super Tx1, Tx1 super Foo, Ty as Ty1, Ty1 as Blah>(
  Ty $val,
  Tx $dummy,
): Tx {
  return $val;
}
function test(BlahBase $blah_base, FooDerived $foo_derived): void {
  bar($foo_derived, $blah_base);
}
