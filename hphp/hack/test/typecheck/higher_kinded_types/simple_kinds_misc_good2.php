<?hh

type ID<T> = T;


function test1<T1,T2<T3>>(T2<T1> $x) : void {}

function test2() : void {
  test1<int, ID>(3);
}
