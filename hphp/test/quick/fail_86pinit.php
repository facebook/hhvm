<?php
function handler() { var_dump(func_get_args()); }
set_error_handler('handler');
class X {
  public $p = Y::FOO;
}
function test() {
  new X;
}
test();
