<?hh
<<file: __EnableUnstableFeatures('no_disjoint_union')>>

interface I {
  abstract const type TNext as I with { type TNext = this::TNext; };
  public function next(): this::TNext;
}

final class C implements I {
  const type TNext = C;
  public function next(): this::TNext {
    return $this;
  }
}

function expectEq<<<__NoDisjointUnion>> T>(T $x, T $y): void {}

function test(I $i): void {
  if ($i is C) {
    return;
  }
  $x = $i->next();
  $y = $x->next();
  $z = $y->next();
  expectEq($y, $x);
  expectEq($z, $x);
}
