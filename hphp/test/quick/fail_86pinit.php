<?hh
function handler(...$args) { var_dump($args); }
set_error_handler('handler');
class X {
  public $p = Y::FOO;
}
function test() {
  new X;
}
test();
