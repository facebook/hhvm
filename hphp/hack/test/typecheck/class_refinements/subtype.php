<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T as mixed;
}

class IntBox {
  const type T = int;
}

class StrBox {
  const type T = string;
}

function is_subtype<Ta, Tb>() : void where Ta as Tb {}

function subtype_tests() : void {

  // Ok below:
  is_subtype<Box with {type T = int}, Box with {type T = int}>();
  is_subtype<Box with {type T = int}, Box>();
  is_subtype<IntBox, Box with {type T = int}>();

  // Errors expected below:
  is_subtype<Box with {type T = int}, Box with {type T = string}>();
  is_subtype<Box, Box with {type T = string}>();
  is_subtype<StrBox, Box with {type T = int}>();

}
