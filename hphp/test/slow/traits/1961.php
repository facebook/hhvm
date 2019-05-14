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
<<__EntryPoint>> function main() {
$o = new MyHelloWorld();
$o->sayHelloWorld();
}
