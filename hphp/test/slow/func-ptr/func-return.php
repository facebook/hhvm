<?hh

function bar(): @string {
  return hh\fun('bar');
}

function foo(): string {
  return hh\fun('foo');
}

<<__EntryPoint>>
function main(): void {
  var_dump(bar());
  var_dump(foo());
}
