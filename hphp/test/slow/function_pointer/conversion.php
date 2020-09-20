<?hh

function foo<reify T>(): void {}

<<__EntryPoint>>
function main(): void {
  $f = foo<int>;

  $g = (int)$f;

  var_dump($g);
}
