<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = I | (function(): int);

interface I {
  public function foo(): int;
}

function f(CT $c): int {
  if ($c is I) {
    return $c->foo();
  } else {
    return $c();
  }
}
