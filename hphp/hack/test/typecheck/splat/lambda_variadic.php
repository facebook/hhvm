<?hh

function f(vec<int> $v): void {
  $f = (...$arg) ==> { throw new Exception(); };
  $g = ($arg) ==> { throw new Exception(); };

  $f(...$v);
  $g(...$v); // error
}
