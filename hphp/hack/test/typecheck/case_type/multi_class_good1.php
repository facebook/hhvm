<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MultiClass = C | D | E ;

interface I {
  public function i(): int;
}
final class C implements I {
  public function g(): int { return 0; }
  public function i(): int { return $this->g(); }
}

final class D implements I {
  public function f(): int { return 1; }
  public function i(): int { return $this->f(); }
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

function impl_interface(MultiClass $x): int {
  if ($x is I) return $x->i();
  return $x->val;
}
