<?hh

// Even with this enabled, we should not be able to do as-tests
<<file:__EnableUnstableFeatures('with_refinement_alias')>>

interface Box {
  abstract const type T;
}
type BoxWithString = Box with { type T = string; };
type BoxWithInt = Box with { type T = int; };
function callee(mixed $b): void {
  $_ = $b as BoxWithString; // ERROR (non-nested)
  $_ = $b as (BoxWithInt, int); // ERROR (nested)
}
