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
class MyClass3 {
  use HelloWorld {
    sayHello as protected final;
  }
  public function sayHi() {
    return $this->sayHello();
  }
}
$a = new MyClass1;
$a->sayHello();
$a = new MyClass2;
$a->sayHello();
$a = new MyClass3;
$a->sayHi();
?>
