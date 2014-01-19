<?php

trait MyTrait {
  public function say_meth() {
    echo "meth: MyTrait\n";
  }
}
class MyBase {
  public function say_meth() {
    echo "meth: MyBase\n";
  }
}
class MyClass {
  use MyTrait;
  public function print_meth() {
    echo "meth: MyClass\n";
  }
}
$o = new MyClass();
$o->print_meth();
