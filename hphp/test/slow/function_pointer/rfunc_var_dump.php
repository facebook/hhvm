<?hh

function foo<reify T>(): void {}

<<__EntryPoint>>
function test(): void {
  $x = vec[foo<int>];
  var_dump($x);
  print_r($x);
}
