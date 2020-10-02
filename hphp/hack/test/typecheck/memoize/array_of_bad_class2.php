<?hh // strict

class Bar {}

<<__Memoize>>
function some_function(varray<Bar> $arg): string {
  return 'hello';
}
