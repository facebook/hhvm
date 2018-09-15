<?hh // strict

class Bar {}

<<__Memoize>>
function some_function(array<Bar> $arg): string {
  return 'hello';
}
