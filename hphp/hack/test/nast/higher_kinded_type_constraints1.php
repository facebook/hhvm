<?hh

// This is the first file out of two to codify the exact scoping behavior of
// (nested) type parameters, including all corner cases.
// You should maintain the property that there is only a single Habstr in the
// resulting NAST file, which means you can just search for "Habstr" in the
// result file to check that it is correct

// This is just to make this a self-contained file without type errors
type T2 = mixed;
type T4 = mixed;
type T6 = mixed;
type T7 = mixed;

class Test<
  Tx as T1, // T1 is in scope: must yield only Habstr in NAST file
  T1<T2 as T7>, // T7 not in scope, must yield Happly referring to alias above
  T3<T4 as T2, // T2 not in scope, must yield Happly referring to alias above
     T5<T6>,
     T7 as T6>, // T6 not in scope, must yield Happly referring to alias above
  T8 as T4, // T4 not in scope, must yield Happly referring to alias above
> {}
