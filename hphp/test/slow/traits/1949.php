<?php

trait SayWorld {
  public function sayHello() {
    echo 'Hello from trait!';
  }
}
class Base {
  public function sayHello() {
    echo 'Hello from Base!';
  }
}
class MyHelloWorld extends Base{
  use SayWorld;
}
$o = new MyHelloWorld();
$o->sayHello();
?>

