<?php
class B {
  public function foo() {
    echo isset($this);
    echo "#\n";
  }
}

function main() {
  $b = new B;
  $b->foo();
}

main();
