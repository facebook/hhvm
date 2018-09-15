<?hh // strict


function foo<T> (T $x) : int where T = int {
  return $x;
}

function equal<T1 as arraykey, T2>(T1 $x, T2 $y) : bool where T1 = T2 {
  return $x == $y;
}

