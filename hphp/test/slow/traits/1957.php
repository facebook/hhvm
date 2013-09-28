<?php

trait MY_TRAIT  {
  public function sayHello() {
    echo 'World!';
  }
}
class MY_CLASS {
  use MY_TRAIT;
}
$MY_OBJ = new MY_CLASS();
$MY_OBJ->sayHello();
?>

