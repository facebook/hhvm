<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type TT = shape('value' => num, 'metric' => string);


function C_reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}

function Vec_filter<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv): bool) $value_predicate = null,
): vec<Tv> {
  return vec[];
}


function testit(
    vec<TT> $v,
    string $s,
  ): num {
    return C_reduce(
      Vec_filter($v, $item ==> $item['metric'] === $s),
      ($sum, $filtered) ==> $sum + $filtered['value'],
      0,
    );
}

function testit2():void {
  $x = 1.0 - C_reduce(
      Map { 2 => 3.4 },
      ($a, $b) ==> $a * $b,
      1.0,
    );
}

function getDuration(Vector<float> $v, float $f): int {
    return (int)C_reduce(
      $v,
      ($sum, $time) ==> { $a = $f * 10; $b = $sum + $a; return $b; },
      0,
    );
  }

function make_sum(darray<int,num> $d): num {
  $n = C_reduce(
    $d,
    ($value, $result) ==> $value + $result,
     0,
      );
  return $n;
}

function make_prod(dict<int,float> $d): num {
  $n = C_reduce(
      $d,
      ($agg, $elem) ==> $agg * $elem,
      1,
    );
  return $n;
}
