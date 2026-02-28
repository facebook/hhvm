<?hh

function foo(): (function(): int) {
  return () ==> 4;
}
