<?php

class Y {
 }
;
class X extends Y {
  function __get($a) {
    var_dump(func_get_args());
    var_dump(func_get_arg(0));
    var_dump(func_num_args());
    return 42;
  }
}
$x = new X;
var_dump($x->buz);
unset($x);
