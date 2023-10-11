<?hh

function foo(): (function(): int) {
  return (...$args) ==> 4;
}
