<?hh

function foo(dict<int, string> $x): dict<int, string> {
  $x[0] = "hello";
  return $x;
}
