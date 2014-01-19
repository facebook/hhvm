<?php

trait MY_TRAIT1 {
  public function sayHello() {
    echo 'Hello from MY_TRAIT1!';
  }
}
trait MY_TRAIT2 {
  public function sayHello() {
    echo 'Hello from MY_TRAIT2!';
  }
}
trait MY_TRAIT3 {
  public function sayHello() {
    echo 'Hello from MY_TRAIT3!';
  }
}
class MyHelloWorld{
  use MY_TRAIT1, MY_TRAIT2, MY_TRAIT3 {
    MY_TRAIT2::sayHello insteadof MY_TRAIT1, MY_TRAIT3;
  }
}
$o = new MyHelloWorld();
$o->sayHello();
?>

