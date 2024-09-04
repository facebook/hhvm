<?hh

interface Box {
  abstract const type T;
}

class C {
  const type TBadInFunArg = (function(Box with { type T = int }): int);
}
