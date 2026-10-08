<?hh

// Same as missing_call.php, but through ResolveFunc (a function pointer).

<<\TrivialHHVMBuiltinWrapper('builtin_that_does_not_exist')>>
function wrapper(): vec<string> {
  return \builtin_that_does_not_exist();
}

<<__EntryPoint>>
function main(): void {
  $f = wrapper<>;
  var_dump($f());
}
