<?php

class Bar {
  public $x = null;
  public function __destruct() {
    $this->x = "dtor";
  }
};

function foo() {
  $a = new Bar();
  $a->x = &$a;
  echo "First: ";
  echo ($a = "something");
  echo "\n";
  echo $a . "\n";
}
foo();
