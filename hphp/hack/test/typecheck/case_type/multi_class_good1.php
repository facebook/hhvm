<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MultiClass = C | D | E ;

final class C {
  public function g(): int { return 0; }
}

final class D {
  public function f(): int { return 1; }
}

final class E {
  public int $val = 1;
}

function impl1(MultiClass $x): int {
  if ($x is C) return $x->g();
  if ($x is D) return $x->f();
  return $x->val;
}

function impl2(MultiClass $x): int {
  if ($x is D) return $x->f();
  if ($x is E) return $x->val;
  return $x->g();
}

function impl3(MultiClass $x): int {
  if ($x is E) return $x->val;
  if ($x is C) return $x->g();
  return $x->f();
}
