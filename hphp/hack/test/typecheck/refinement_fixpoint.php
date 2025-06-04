<?hh
<<file: __EnableUnstableFeatures('no_disjoint_union')>>

interface I {
  abstract const type TNext as I with { type TNext = this::TNext; };
  public function next(): this::TNext;
}

function expectEq<<<__NoDisjointUnion>> T>(T $x, T $y): void {}

function test(I $i): void {
  $x = $i->next();
  $y = $x->next();
  $z = $y->next();
  expectEq($y, $x);
  expectEq($z, $x);
}
