<?php

trait MY_TRAIT1 {
  public function sayHello() {
    echo 'Hello from MY_TRAIT1!';
  }
}
class MyHelloWorld{
  use MY_TRAIT1 {
    MY_TRAIT1::sayHello as falaOi;
  }
}
$o = new MyHelloWorld();
$o->falaOi();
?>

