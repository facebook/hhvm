<?php

trait SayWorld {
  public function sayHelloWorld() {
    self::sayHello();
    echo 'World!';
  }
}
class MyHelloWorld {
  use SayWorld;
  public function sayHello() {
    echo 'Hello ';
  }
}
$o = new MyHelloWorld();
$o->sayHelloWorld();
?>

