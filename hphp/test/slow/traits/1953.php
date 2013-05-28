<?php

trait MY_TRAIT2 {
  protected function sayGoodbye() {
    echo "Goodbye from MY_TRAIT2!\n";
  }
}
class MyHelloWorld{
  use MY_TRAIT2 {
    MY_TRAIT2::sayGoodbye as public falaTchau;
  }
}
$o = new MyHelloWorld();
$o->falaTchau();
?>
