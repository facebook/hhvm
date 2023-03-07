<?hh
<<__EntryPoint>>
function entrypoint_autoload_case(): void {
  TestA::$D = 1;
  var_dump(TestA::$D);
}
