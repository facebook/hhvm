<?php

trait my_trait {
  abstract function foo();
  public function bar() {
    echo "I am bar\n";
    self::foo();
  }
}
class my_class {
  use my_trait;
  private function foo() {
    echo "I am foo\n";
  }
}
$o = new my_class;
$o->bar();
