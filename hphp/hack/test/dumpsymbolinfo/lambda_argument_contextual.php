<?hh

function map<Tv1, Tv2>(vec<Tv1> $v, (function(Tv1): Tv2) $f): vec<Tv2> {
  return vec[];
}

function test(): void {
  map(vec[1, 2, 3], $n ==> $n));
}
