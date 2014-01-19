<?php

trait HelloWorld {
  private function sayHello() {
    echo "Hello World 1!\n";
  }
}
trait HelloWorld2 {
  public function sayHello2() {
    echo "Hello World 2!\n";
  }
}
class MyClass1 {
  use HelloWorld2 {
    sayHello as public;
  }
  use HelloWorld;
}
$a = new MyClass1;
$a->sayHello();
$a->sayHello2();
?>
