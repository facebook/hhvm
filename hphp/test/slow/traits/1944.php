<?php

trait HelloWorld {
  public function sayHello() {
    echo "Hello World!\n";
  }
}
class TheWorldIsNotEnough {
  use HelloWorld;
  public function sayHello() {
    echo "Hello Universe!\n";
  }
}
<<__EntryPoint>> function main() {
$o = new TheWorldIsNotEnough();
$o->sayHello();
}
