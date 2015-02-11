<?php

class X {
  private $i;
  public function __construct($i) { $this->i = $i; }
  public function __destruct() { echo "dtor: $this->i\n"; }
}

function err() {}

function foo() {
  set_error_handler('err');
  hash(new X(1), new X(2));
}
foo();
