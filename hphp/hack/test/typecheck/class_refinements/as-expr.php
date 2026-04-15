<?hh

// Even with this enabled, we should not be able to do as-tests
<<file:__EnableUnstableFeatures('with_refinement_alias')>>

interface Box {
  abstract const type T;
}

function callee(mixed $b): void {
  $_ = $b as Box with { type T = string }; // ERROR (non-nested)
  $_ = $b as (Box with { type T = int }, int); // ERROR (nested)
}
