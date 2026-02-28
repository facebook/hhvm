<?hh

interface IHasTypeConst {
  abstract const type Tconst;
}

function foo<T1, T2>(): void
where
  T1 = T2,
  // the particular constraint is not important, just that it has a refinement
  // that isn't already satisfiable by T1 or T2
  T1 as IHasTypeConst with { type Tconst as mixed; } {
}
