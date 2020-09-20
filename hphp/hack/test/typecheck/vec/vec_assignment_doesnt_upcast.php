<?hh // partial

function foo(vec<string> $x): vec<string> {
  $x[0] = "hello";
  return $x;
}
