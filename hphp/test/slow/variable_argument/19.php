<?php

class A {
 public function __construct($a) {
  var_dump(func_num_args());
  var_dump(func_get_args());
}
}

 <<__EntryPoint>>
function main_19() {
$obj = new A(1, 2, 3);
 $obj = new A('test');
}
