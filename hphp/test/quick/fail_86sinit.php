<?hh
function handler(...$args) { var_dump($args); }
set_error_handler('handler');
class X {
  public static $s = Z::BAR;
}
function test() {
  var_dump(X::$s);
}
test();
