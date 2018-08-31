<?php

class C extends ArrayObject {
  public function __construct($arg) {
    // ArrayObject::__construct hasn't run
    var_dump(isset($this[$arg]));
  }
}


<<__EntryPoint>>
function main_access_no_ctor() {
$c = new C(3);
}
