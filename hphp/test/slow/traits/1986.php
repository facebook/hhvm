<?php

trait Hello {
  public function sayHello() {
    echo "Hello\n";
  }
}
trait Hello1 {
  public function sayNum() {
    echo "1\n";
  }
}
trait Hello2 {
  public function sayNum() {
    echo "2\n";
  }
}
class MyClass {
  use Hello1, Hello2 {
    Hello1::sayNum insteadof Hello2;
  }
  use Hello2 {
  }
}
$o = new MyClass();
$o->sayNum();
