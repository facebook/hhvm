<?hh // partial

function g(?int $x) {}

function f(mixed $x) {
  $x as ?string;
  g($x);
}
