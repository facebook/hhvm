<?hh

newtype Test1<T1> = int;

newtype Test2<T1, T2<T3>> = int;

type TestCase1 = Test2<int, Test1>; //ok

type TestCase2 = Test2<int>; // arity mistmatch: too few

type TestCase3 = Test2<int, Test1, int>; // arity mistmatch: too many

type TestCase4 = Test2<int, int>; // kind mismatch
