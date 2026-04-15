<?hh

// Even with this enabled, we should not be able to do is-tests
<<file:__EnableUnstableFeatures('with_refinement_alias')>>

interface Box {
  abstract const type T;
}

function is_contains_box_refinement(mixed $b): bool {
  return ($b is Box with { type T = string }) // ERROR (non-nested)
    ||
    ($b is shape('bad' => Box with { type T = int })); // ERROR (nested)
}
