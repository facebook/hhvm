<?php

abstract class B {
  private function priv() { echo "B::priv\n"; }
  function func(){
    $this->priv();
    var_dump(get_class_methods($this));
  }
}

class C extends B {
  private function priv() { echo "C::priv\n"; }
}

$obj = new C();
$obj->func();
