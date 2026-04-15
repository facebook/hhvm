<?hh

interface Box {
  abstract const type T;
}

interface Bad {
  const type TBad = Box with { type T = int }; // ERROR (refinements only allowed under flag)

  const type TBadNested = (int, Box with { type T = int }); // ERROR

  abstract const type TBadInPartiallyAbstractRHS = shape(
    'nested' => Box with { type T = int }, // ERROR
  );
}

interface Good {
  abstract const type TOkInAs as Box with { type T = int };

  abstract const type TOkInPartiallyAbstractLHS as Box with { type T = int } =
    IntBox;
}

class IntBox implements Box {
  const type T = int;
}
