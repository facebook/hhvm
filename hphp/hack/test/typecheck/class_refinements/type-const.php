<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

interface Bad {
  const type TBad = Box with { type T = int };  // ERROR

  const type TBadNested = (int, Box with { type T = int });  // ERROR

  abstract const type TBadInPartiallyAbstractRHS = shape(
    'nested' => Box with { type T = int }  // ERROR
  );
}

interface Good {
  abstract const type TOkInAs as Box with { type T = int };

  abstract const type TOkInPartiallyAbstractLHS as Box with { type T = int } = IntBox;
}

class IntBox implements Box { const type T = int; }
