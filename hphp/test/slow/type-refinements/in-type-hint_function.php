<?hh
<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T as mixed;
}

function as_argument(Box with { type T = int } $arg): void {}

function in_return(): Box with { type T = int } {
  return new IntBox();
}

function in_return_higher_order(): (function(): Box with { type T = int }) {
  return () ==> new IntBox();
}

class IntBox extends Box { const type T = int; }
