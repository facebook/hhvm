<?hh


class Test1<T> {}

function test2<T1, T2<T3>>() : void {}

function tests() : void {
  test2<int, Test1>(); //ok

  test2<int>(); // arity mistmatch: too few

  test2<int, Test1, int>(); // arity mistmatch: too many

  test2<int, int>(); // kind mismatch
}
