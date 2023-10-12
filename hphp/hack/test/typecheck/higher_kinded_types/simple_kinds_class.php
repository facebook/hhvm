<?hh // strict

class Test1<T1> {}

class Test2<T1, T2<T3>> {}

type TestCase1 = Test2<int, Test1>; //ok

type TestCase2 = Test2<int>; // arity mistmatch: too few

type TestCase3 = Test2<int, Test1, int>; // arity mistmatch: too many

type TestCase4 = Test2<int, int>; // kind mismatch


function test() : void {
  new Test2<int, Test1>(); // ok

  new Test2<int, Test2>(); // kind mismatch
}
