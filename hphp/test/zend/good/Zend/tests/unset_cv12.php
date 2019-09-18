<?hh
function foo() {unset($GLOBALS["x"]);}
<<__EntryPoint>>
function main_entry(): void {
  $x = 1;
  call_user_func(fun("foo"));
  echo "ok\n";
}
