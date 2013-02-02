<?php

class A {
  public function f($x) {
    var_dump($x());
    var_dump(get_class());
  }
}

$a = new A();
$a->f('get_class');
var_dump(get_class($a));
var_dump(get_class());
var_dump(get_class(null));
