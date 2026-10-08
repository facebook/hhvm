<?hh

// The wrapped function is not defined in this process, e.g. because the
// wrapper was generated against a newer HHVM. Calling the wrapper must raise
// the usual undefined-function error instead of dispatching to a null Func.

<<\TrivialHHVMBuiltinWrapper('builtin_that_does_not_exist')>>
function wrapper(): vec<string> {
  return \builtin_that_does_not_exist();
}

<<__EntryPoint>>
function main(): void {
  var_dump(wrapper());
}
