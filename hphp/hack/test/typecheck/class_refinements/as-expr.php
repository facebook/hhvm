<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

function callee(mixed $b): void {
  $_ = $b as Box with {type T = string};  // ERROR (non-nested)
  $_ = $b as (Box with {type T = int}, int);  // ERROR (nested)
}
