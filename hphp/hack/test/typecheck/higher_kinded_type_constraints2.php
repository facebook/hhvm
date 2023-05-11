<?hh

// This is the second file out of two to codify the exact scoping behavior of
// (nested) type parameters, including all corner cases.
// You should maintain the property that there is only a single Happly in the
// resulting NAST file, which means you can just search for "Happly" in the
// result file to check that it is correct


// This is just to make this a self-contained file without type errors
type T7 = mixed;

class Test<
  T1 as T8, // T8 in scope, must yield Habstr
  T2<
    T3 as T1, // T1 in scope, must yield Habstr
    T4 as T2, // T2 in scope, must yield Habstr
    T5 as T4, // T4 in scope, must yield Habstr
    T6 as T7, // T7 in scope, must yield Habstr
    T7<T8 as T9>, // T9 in scope, must yield Habstr
    T9>
    as (T3, T9),  // T3, T9 in scope, must yield Habstr
  T8 as T2, // T2 in scope, must yield Habstr
  Tx as T3 // T3 refers to typedef above, not the tparam T3,
           // must yield only Happly in the restult NAST file
> {}
