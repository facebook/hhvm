<?hh // strict

function foo(): (function(): int) {
  return () ==> 4;
}
