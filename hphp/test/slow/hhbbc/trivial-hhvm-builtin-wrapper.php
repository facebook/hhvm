<?hh

<<\TrivialHHVMBuiltinWrapper("bar")>>
function foo(): string {
  return __FUNCTION__;
}

function bar(): string{
  return __FUNCTION__;
}

<<__EntryPoint>>
function main(): void {
  var_dump(foo());
}
