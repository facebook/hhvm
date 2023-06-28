<?hh
function handler(...$args) :mixed{ var_dump($args); }
class X {
  public static $s = Z::BAR;
}
function test() :mixed{
  var_dump(X::$s);
}
<<__EntryPoint>>
function entrypoint_fail_86sinit(): void {
  set_error_handler(handler<>);
  test();
}
