<?hh

function bar(): <<__Soft>> string {
  return bar<>;
}

function foo(): string {
  return foo<>;
}

<<__EntryPoint>>
function main(): void {
  var_dump(bar());
  var_dump(foo());
}
