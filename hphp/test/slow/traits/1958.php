<?php

trait MY_TRAIT1 {
  public function sayHello() {
    echo "Hello from MY_TRAIT1\n";
  }
}
trait MY_TRAIT2 {
  use MY_TRAIT1;
  public function sayHello() {
    echo "Hello from MY_TRAIT2\n";
  }
}
class MY_CLASS {
  use MY_TRAIT2;
}
$o = new MY_CLASS;
$o->sayHello();
?>
