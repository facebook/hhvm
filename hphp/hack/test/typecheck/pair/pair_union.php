<?hh

function id<T>(T $x): T {
  return $x;
}

function apply<Ta, Tb>(Ta $x, (function(Ta): Tb) $f): Tb {
  return $f($x);
}

function test(bool $x): Pair<arraykey, int> {
  return apply(
    $x,
    $x ==> {
      if ($x) {
        return Pair { 42, 1 };
      }
      return Pair { id('foo'), 2 };
    },
  );
}
