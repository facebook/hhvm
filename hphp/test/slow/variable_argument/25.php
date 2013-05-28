<?php

class Y {
 function __destruct() {
 }
 }
;
class X extends Y {
  function __get($a) {
    var_dump(func_get_args());
    var_dump(func_get_arg(0));
    var_dump(func_num_args());
    return 42;
  }
  function __destruct() {
    var_dump(func_get_args());
    var_dump(func_get_arg(0));
    var_dump(func_num_args());
    return 2442;
  }
}
$x = new X;
var_dump($x->buz);
unset($x);
