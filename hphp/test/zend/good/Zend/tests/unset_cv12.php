<?hh
function foo() :mixed{\HH\global_unset("x");}
<<__EntryPoint>>
function main_entry(): void {
  $x = 1;
  call_user_func(foo<>);
  echo "ok\n";
}
