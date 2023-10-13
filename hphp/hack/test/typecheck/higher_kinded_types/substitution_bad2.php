<?hh

type ID<T> = T;
type Apply<TF<_>, TA> = TF<TA>;

function test1(Apply<ID,int> $x) : Apply<ID,string> {
  return $x;
}

function test2(ID<int> $x) : ID<string> {
  return $x;
}
