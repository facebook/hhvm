<?php

class A {
  public function __destruct() {
    restore_error_handler();
  }
  public function __construct() {
    set_error_handler(array($this, "callback"));
  }
  public function callback($errno, $msg) {
    echo "my logger: ". $msg ."\n";
    return true;
  }
}


<<__EntryPoint>>
function main_3329() {
$a = new A;
trigger_error("blarg");
unset($a);
}
