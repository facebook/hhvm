<?hh

// Even with this enabled, we should not be able to do is-tests
<<file:__EnableUnstableFeatures('with_refinement_alias')>>

interface Box {
  abstract const type T;
}
type BoxWithString = Box with { type T = string; };
type BoxWithInt = Box with { type T = int; };
abstract class C {
  const type TS = Box with { type T = string; };
  const type TI = Box with { type T = int; };
  <<__Enforceable>>
  abstract const type TC as Box with { type T = arraykey; };
}

function is_contains_box_refinement(mixed $b): bool {
  return ($b is BoxWithString) // ERROR (non-nested)
    ||
    ($b is shape('bad' => BoxWithInt)); // ERROR (nested)
}
function is_contains_box_refinement2(mixed $b): bool {
  return ($b is C::TS) // ERROR (non-nested)
    ||
    ($b is shape('bad' => C::TI)); // ERROR (nested)
}
abstract class D extends C {
  public function is_contains_box_refinement3(mixed $b): bool {
    return ($b is this::TC) // OK (bound on abstract type constant)
    ||
    ($b is shape('bad' => this::TC)); // OK (bound on abstract type constant)
  }
}
