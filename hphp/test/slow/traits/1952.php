<?php

trait MY_TRAIT1 {
  public function sayHello() {
    echo "Hello from MY_TRAIT1!\n";
  }
}
trait MY_TRAIT2 {
  use MY_TRAIT1;
  public function sayGoodbye() {
    echo "Goodbye from MY_TRAIT2!\n";
  }
}
class MyHelloWorld{
  use MY_TRAIT2 {
    MY_TRAIT2::sayHello as falaOi;
    MY_TRAIT2::sayGoodbye as falaTchau;
  }
}
$o = new MyHelloWorld();
$o->falaOi();
$o->falaTchau();
$o->sayHello();
$o->sayGoodbye();
?>
