<?hh

// two generics so that we see how many errors we get
type HasBoundedGeneric<T1 as string, T2 as int> = null;
