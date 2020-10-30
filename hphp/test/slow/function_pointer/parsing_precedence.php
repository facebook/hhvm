<?hh

function foo<T>(): void {}

<<__EntryPoint>>
function test(): void {
  $f = 1;
  // This used to parse as: `$f === (foo<int> ? 1 : 0`
  var_dump($f === foo<int> ? 1 : 0);
}
