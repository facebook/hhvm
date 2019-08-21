<?hh

function f() {
  call_user_func(fun('f'));
}
<<__EntryPoint>> function main(): void {
call_user_func(fun('f'));
}
