<?php

class C extends ArrayObject {
  public function __construct($arg) {
    // ArrayObject::__construct hasn't run
    var_dump(isset($this[$arg]));
  }
}

$c = new C(3);
