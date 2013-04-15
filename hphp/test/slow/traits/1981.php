<?php


trait HelloWorld {
  private function sayHello() {
    echo "Hello World!\n";
  }
}
class MyClass1 {
  use HelloWorld {
    sayHello as public;
  }
}
class MyClass2 {
  use HelloWorld {
    sayHello as final;
  }
}
$a = new MyClass1;
$a->sayHello();
$a = new MyClass2;
$a->sayHello();
?>
