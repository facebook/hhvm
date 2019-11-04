<?hh

function bar(): @string {
  return HH\fun('bar');
}

function foo(): string {
  return HH\fun('foo');
}

<<__EntryPoint>>
function main(): void {
  var_dump(bar());
  var_dump(foo());
}
