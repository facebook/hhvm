<?php

class A {
 public function test($a) {
  var_dump(func_num_args());
  var_dump(func_get_args());
}
}
 $obj = new A();
 $obj->test('test');
 $obj->test(1, 2, 3);
