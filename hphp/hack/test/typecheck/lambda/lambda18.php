<?hh

function foo(): (function(mixed...): int) {
  return (mixed ...$args) ==> 4;
}
