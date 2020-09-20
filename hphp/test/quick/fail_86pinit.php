<?hh
function handler(...$args) { var_dump($args); }
class X {
  public $p = Y::FOO;
}
function test() {
  new X;
}
<<__EntryPoint>>
function entrypoint_fail_86pinit(): void {
  set_error_handler(fun('handler'));
  test();
}
