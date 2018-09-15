<?hh // strict

function foo(): (function(): int) {
  return (...$args) ==> 4;
}
