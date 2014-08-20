<?php

var_dump(ini_get('hhvm.enable_xhp'));

class :foo {
  public function __toString() {
    return "it works";
  }
}
var_dump((string) <foo />);
