<?hh
function handler(...$args) :mixed{ var_dump($args); }
class X {
  public $p = Y::FOO;
}
function test() :mixed{
  new X;
}
<<__EntryPoint>>
function entrypoint_fail_86pinit(): void {
  set_error_handler(handler<>);
  test();
}
