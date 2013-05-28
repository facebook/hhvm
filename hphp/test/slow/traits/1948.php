<?php

trait SayWorld {
  public function sayHello() {
    echo 'Hello from trait!';
  }
}
class MyHelloWorld {
  public function sayHello() {
    echo 'Hello from class!';
  }
  use SayWorld;
}
$o = new MyHelloWorld();
$o->sayHello();
?>

