<?hh

function f() :mixed{
  call_user_func(f<>);
}
<<__EntryPoint>> function main(): void {
call_user_func(f<>);
}
