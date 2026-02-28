<?hh

class C {
  public ?int $i;
}

class S {
  public static ?C $c;
}

function global_static_aliasing(C $alias): int {
  $alias->i = 42;

  $objC = S::$c;
  invariant($objC is nonnull, "");
  $objC->i = null;
  // All properties with ->i should be invalidated here.

  return $alias->i; // Shouldn't type check
}

// The following is to show how aliasing arises
function global_static_aliasing_callsite(): void {
  S::$c = new C();
  global_static_aliasing(S::$c);
}
