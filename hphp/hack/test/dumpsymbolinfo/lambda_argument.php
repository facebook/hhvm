<?hh // strict

function map<Tv1, Tv2>(vec<Tv1> $v, (function(Tv1): Tv2) $f): vec<Tv2> {
  return vec[];
}

function test(): void {
  $f = $nums ==> {
    $v = map($nums, $n ==> $n);
  };
  $f(vec[1, 2, 3]);
}
