<?php

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

<<__EntryPoint>>
function main_2003() {
var_dump(class_uses('MyClass'));
}
