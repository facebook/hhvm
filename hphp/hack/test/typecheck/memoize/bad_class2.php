<?hh // strict

class Bar {}

<<__Memoize>>
function some_function(Bar $arg): string {
  return 'hello';
}
