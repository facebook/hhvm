<?php

trait Hello {
  public function sayHello() {
    echo 'Hello ';
  }
}
trait World {
  public function sayWorld() {
    echo 'World';
  }
  use Hello;
}
class MyHelloWorld {
  use World;
  public function sayExclamationMark() {
    echo "!\n";
  }
}
$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
$o->sayExclamationMark();
