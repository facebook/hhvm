<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T as mixed;
}

function is_subtype<Ta, Tb>() : void where Ta as Tb {}

function subtype_tests() : void {
  is_subtype<Box with {type T = int}, Box>(); // OK
}

interface BoxController<T as Box with { type T = int }> { // OK
  function swap<T as Box with { type T = int }>(T $other): void; // OK
}
