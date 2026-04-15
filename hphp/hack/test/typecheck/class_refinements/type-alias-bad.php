<?hh

interface Box {
  abstract const type T;
}

type TBad =
  Box with { type T = int }; // ERROR (refinements only allowed under flag)

type TBadNested = shape(
  'in_shape' => Box with { type T = string }, // ERROR
  'in_tparam' => vec<Box with { type T = bool }>, // ERROR
);
