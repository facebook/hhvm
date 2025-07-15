<?hh
<<file: __EnableUnstableFeatures('type_splat', 'open_tuples')>>

class Foo<T as (mixed...)> {
  public function expectFun((function(...T): void) $f): void {}
}

function test_splat_in_expected(): void {
  $f = new Foo<(arraykey)>();
  $g = (arraykey $x) ==> {};
  // This works
  $f->expectFun($g);
  // This does not
  $f->expectFun((arraykey $x) ==> {});
}
