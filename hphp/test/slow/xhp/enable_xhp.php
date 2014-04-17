<?php

var_dump(ini_get('hhvm.eval.enable_xhp'));

class :foo {
  public function __toString() {
    return "it works";
  }
}
var_dump((string) <foo />);
