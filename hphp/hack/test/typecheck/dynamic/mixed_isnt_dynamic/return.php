<?hh // strict

function foo(dynamic $x): mixed {
  return $x;
}

function bar(mixed $x): dynamic {
  return $x;
}
