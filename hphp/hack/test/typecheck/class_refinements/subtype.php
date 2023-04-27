<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

class C1 {}
class C2 extends C1 {}
class C3 extends C2 {}
class C4 extends C3 {}

abstract class Box {
  abstract const type T as mixed;
}

class IntBox extends Box {
  const type T = int;
}

class StrBox extends Box {
  const type T = string;
}

// We ignore the NastCheck
type RngBox<Tlo, Thi> = Box with {type T super Tlo as Thi};

function is_subtype<Ta, Tb>() : void where Ta as Tb {}

function subtype_tests() : void {

  // Ok below:
  is_subtype<Box with {type T = int}, Box with {type T = int}>();
  is_subtype<Box with {type T = int}, Box>();
  is_subtype<IntBox, Box with {type T = int}>();
  is_subtype<RngBox<C3, C2>, RngBox<C4, C1>>();

  // Errors expected below:
  is_subtype<Box with {type T = int}, Box with {type T = string}>();
  is_subtype<Box, Box with {type T = string}>();
  is_subtype<StrBox, Box with {type T = int}>();
  is_subtype<RngBox<C4, C2>, RngBox<C3, C2>>();

}
