<?hh // strict

function foo(): (function(...): int) {
  return (mixed ...$args) ==> 4;
}
