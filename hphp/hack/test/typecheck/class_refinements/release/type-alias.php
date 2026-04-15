<?hh

interface Box {
  abstract const type T;
}

type TBad = Box with { type T = int }; // OK (refinements now allowed)

type TBadNested = shape(
  'in_shape' => Box with { type T = string }, // OK
  'in_tparam' => vec<Box with { type T = bool }>, // OK
);
