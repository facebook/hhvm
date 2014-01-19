<?php
function handler() { var_dump(func_get_args()); }
set_error_handler('handler');
class X {
  public static $s = Z::BAR;
}
function test() {
  var_dump(X::$s);
}
test();
