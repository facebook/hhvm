<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

type TBad = Box with { type T = int };  // ERROR

type TBadNested = shape(
  'in_shape' => Box with { type T = string },  // ERROR
  'in_tparam' => vec<Box with { type T = bool }>,  // ERROR
);

type TGood<T as Box with { type T = int }> = Covar<T>;  // OK
