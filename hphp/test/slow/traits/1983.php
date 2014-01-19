<?php

class Base {
  public function sayHello() {
    echo 'Hello ';
  }
}
class MyHelloWorld extends Base {
  use SayWorld;
}
trait SayWorld {
  public function sayHello() {
    parent::sayHello();
    echo 'World!';
  }
}
$o = new MyHelloWorld();
$o->sayHello();
?>

