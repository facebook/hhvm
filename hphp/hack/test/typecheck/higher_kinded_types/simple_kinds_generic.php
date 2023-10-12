<?hh // strict

class C<T> {}

function test<T1, T2<_>>() : void {}

function tests<Test1<T1>,Test2<T2, T4<_>>>() : void {
  $f = (Test1<int> $x) ==> {}; //ok

  $f = (Test1 $x) ==> {}; // arity mistmatch: too few

  $f = (Test2<int, C, int> $x) ==> {}; // arity mistmatch: too many

  $f = (Test2<int, int> $x) ==> {}; // kind mistmatch (1)

  $f = (Test2<int, Test2> $x) ==> {}; // kind mistmatch (2)
}
