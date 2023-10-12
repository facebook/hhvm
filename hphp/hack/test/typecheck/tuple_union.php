<?hh // strict

function id<T>(T $x): T {
  return $x;
}

function apply<Ta, Tb>(Ta $x, (function(Ta): Tb) $f): Tb {
  return $f($x);
}

function test(bool $x): (arraykey, int) {
  return apply(
    $x,
    $x ==> {
      if ($x) {
        return tuple(42, 1);
      }
      return tuple(id('foo'), 2);
    },
  );
}
