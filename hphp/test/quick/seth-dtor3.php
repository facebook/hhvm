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
  if ($a) {
    echo "Value: ";
  }
  $a = "something";
  echo $a . "\n";
}
foo();
