<?hh
function handler(...$args) { var_dump($args); }
set_error_handler(fun('handler'));
class X {
  public static $s = Z::BAR;
}
function test() {
  var_dump(X::$s);
}
test();
