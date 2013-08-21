<?php

abstract class B {
  private function priv() { }
  function func(){
    $this->priv();
    var_dump(get_class_methods($this));
  }
}

class C extends B {}

$obj = new C();
$obj->func();
